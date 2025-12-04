#!/usr/bin/env php
<?php
declare(strict_types=1);

const REQUEST_REPLY_TIMEOUT = 300;

function microtime_float()
{
    list($usec, $sec) = explode(" ", microtime());
    return ((float)$usec + (float)$sec);
}

$context = new ZMQContext();
$requester = new ZMQSocket($context, ZMQ::SOCKET_REQ);
$requester->setSockOpt(ZMQ::SOCKOPT_LINGER, REQUEST_REPLY_TIMEOUT);
$requester->setSockOpt(ZMQ::SOCKOPT_SNDTIMEO, REQUEST_REPLY_TIMEOUT);
$requester->setSockOpt(ZMQ::SOCKOPT_RCVTIMEO, REQUEST_REPLY_TIMEOUT);
$requester->connect("ipc:///tmp/root-serialPmodInterfaceAPI");

$o = new StdClass();
$o->messageType = 'serialRequest';
$o->data = "00,00,03,00,02,01";
$o->timeoutMessageMs = 20;
$o->timeoutByteMs = 5;

$jsonStr = json_encode($o);

$start_time = microtime_float();

echo "Sending: $jsonStr" . PHP_EOL;

$requester->send($jsonStr);
$respData = $requester->recv();

$end_time = microtime_float();

var_dump($respData);

$run_time = $end_time - $start_time;

echo "Run Time: $run_time" . PHP_EOL;
