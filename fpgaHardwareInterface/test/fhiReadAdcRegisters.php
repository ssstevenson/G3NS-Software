#!/usr/bin/env php
<?php
declare(strict_types=1);
require_once(__DIR__ . '/../helpers/SocketCoordinatorSingleton.php');

const REQUEST_REPLY_TIMEOUT = 1000;

$context = new ZMQContext();

echo 'Requesting snapshot...' . PHP_EOL;
$requester = new ZMQSocket($context, ZMQ::SOCKET_REQ);
$requester->setSockOpt(ZMQ::SOCKOPT_LINGER, REQUEST_REPLY_TIMEOUT);
$requester->setSockOpt(ZMQ::SOCKOPT_SNDTIMEO, REQUEST_REPLY_TIMEOUT);
$requester->setSockOpt(ZMQ::SOCKOPT_RCVTIMEO, REQUEST_REPLY_TIMEOUT);
$requester->connect(SocketCoordinatorSingleton::getInstance()->getSocketName('getFpgaHardwareInterfaceAPI'));
$request = new StdClass();
$request->messageType = 'fpgaRequestWrite';
$request->hexAddress = '80100008';
$request->data = '1';
$requestString = json_encode($request);
echo 'Sending: ' . $requestString . PHP_EOL;
$requester->send($requestString);
$reply = $requester->recv();
if ($reply != false && strlen($reply) > 0) {
    echo $reply . PHP_EOL . PHP_EOL;
    //print_r(json_decode($reply, true));
} else {
    $context = null;
    echo "ERROR.  Timeout occurred." . PHP_EOL;
    exit(-1);
}

echo "Dumping ADC registers..." . PHP_EOL;
$request = new StdClass();
$request->messageType = 'fpgaRequestReadRange';
$request->startHexAddress = '8010000C';
$request->endHexAddress = '80100048';
$requestString = json_encode($request);
echo 'Sending: ' . $requestString . PHP_EOL;
$requester->send($requestString);
$reply = $requester->recv();
if ($reply != false && strlen($reply) > 0) {
    echo $reply . PHP_EOL . PHP_EOL;
    print_r(json_decode($reply, true));
} else {
    $context = null;
    echo "ERROR.  Timeout occurred." . PHP_EOL;
    exit(-1);
}
