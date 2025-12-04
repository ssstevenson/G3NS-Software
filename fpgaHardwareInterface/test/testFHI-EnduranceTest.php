#!/usr/bin/env php
<?php
declare(strict_types=1);
require_once(__DIR__ . '/../helpers/SocketCoordinatorSingleton.php');

function stdDev(array $arr): array
{
    $num_of_elements = count($arr);

    if ($num_of_elements <= 0)
    {
        return array(0, 0, 0);
    }

    $variance = 0.0;
    $average = array_sum($arr)/$num_of_elements;

    foreach ($arr as $i)
    {
        $variance += pow(($i - $average), 2);
    }

    $standandard_deviation = (float)sqrt($variance/$num_of_elements);

    return array($standandard_deviation, $average, $num_of_elements);
}

function createZmqSocket(&$context, &$socket)
{
    if (isset($socket))
    {
        unset($socket);
        unset($context);
        sleep(1);
        $context = new ZMQContext();
        sleep(1);
    }
    $socket = new ZMQSocket($context, ZMQ::SOCKET_REQ);
    $socket->setSockOpt(ZMQ::SOCKOPT_LINGER, REQUEST_REPLY_TIMEOUT);
    $socket->setSockOpt(ZMQ::SOCKOPT_SNDTIMEO, REQUEST_REPLY_TIMEOUT);
    $socket->setSockOpt(ZMQ::SOCKOPT_RCVTIMEO, REQUEST_REPLY_TIMEOUT);
    $socket->connect(SocketCoordinatorSingleton::getInstance()->getSocketName('getFpgaHardwareInterfaceAPI'));
}

const REQUEST_REPLY_TIMEOUT = 500;
const LOOPBACK_ADDRESS = '80000008';

$context = new ZMQContext();
createZmqSocket($context, $requester);

$dataCnt = 0;
$stepCnt = 0;

$writeObj = new StdClass();
$writeObj->messageType = 'fpgaRequestWrite';
$writeObj->hexAddress = LOOPBACK_ADDRESS;
// $writeObj->messageType = 'fpgaModuleRequestWrite';
// $writeObj->module = 'INVENTORY';
// $writeObj->hexOffset = '0008';

$readObj = new StdClass();
$readObj->messageType = 'fpgaRequestRead';
$readObj->hexAddress = LOOPBACK_ADDRESS;
// $readObj->messageType = 'fpgaModuleRequestRead';
// $readObj->module = 'INVENTORY';
// $readObj->hexOffset = '0008';

$printTime = microtime(true);
$writeTimes = array();
$readTimes = array();

while(true)
{
    $writeObj->data = dechex($dataCnt);

    try
    {
        $writeStart = microtime(true);
        $requester->send(json_encode($writeObj));
        $writeReply = $requester->recv();
        // echo 'Write Response: ' . $reply . PHP_EOL;
        $writeEnd = microtime(true);

        $readStart = microtime(true);
        $requester->send(json_encode($readObj));
        $readReply = $requester->recv();
        // echo 'Read Response: ' . $reply . PHP_EOL;
        $readEnd = microtime(true);

        array_push($writeTimes, ($writeEnd - $writeStart));
        array_push($readTimes, ($readEnd - $readStart));
    }
    catch (Exception $e)
    {
        createZmqSocket($context, $requester);
        $writeTimes = array();
        $readTimes = array();
        $printTime = microtime(true);
        echo $stepCnt . ' ...' . PHP_EOL;
        $stepCnt += 1;
    }

    $curTime = microtime(true);
    if (($curTime - $printTime) > 1.0)
    {
        $printTime = $curTime;
        list($writeStdDev, $writeAverage, $writeCnt) = stdDev($writeTimes);
        list($readStdDev, $readAverage, $readCnt) = stdDev($readTimes);

        $writeTimes = array();
        $readTimes = array();

        echo $stepCnt . ' W ' . $writeCnt . ' ' . $writeAverage . ' ' . $writeStdDev . ' -- R ' . $readCnt . ' ' .
            $readAverage . ' ' . $readStdDev . PHP_EOL . $writeReply . PHP_EOL . $readReply . PHP_EOL;

        $stepCnt += 1;
    }

    $dataCnt = ($dataCnt + 1) & 0xFFFFFFFF;
}
