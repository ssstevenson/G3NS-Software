#!/usr/bin/env php
<?php
declare(strict_types=1);
require_once(__DIR__ . '/../helpers/SocketCoordinatorSingleton.php');

const REQUEST_REPLY_TIMEOUT = 500;

$context = new ZMQContext();

$requester = new ZMQSocket($context, ZMQ::SOCKET_REQ);
$requester->setSockOpt(ZMQ::SOCKOPT_LINGER, REQUEST_REPLY_TIMEOUT);
$requester->setSockOpt(ZMQ::SOCKOPT_SNDTIMEO, REQUEST_REPLY_TIMEOUT);
$requester->setSockOpt(ZMQ::SOCKOPT_RCVTIMEO, REQUEST_REPLY_TIMEOUT);
$requester->connect(SocketCoordinatorSingleton::getInstance()->getSocketName('getStatusProcessorAPI'));

// Build a request object
$o = new StdClass();
$o->messageType = 'getStatusItemBatch';
// $o->keys = ['test', 'test2', 'test3', 'test4', 'test5'];
$o->keys = ['ENV_COOLING_TYPE', 'ENV_COOLING_MODE', 'ENV_MAX_TEMP', 'ENV_SETPOINT', 'PA_1_TEMP'];

$startTime = microtime(true);

$requester->send(json_encode($o));
$reply = $requester->recv();

$endTime = microtime(true);

if ($reply != false && strlen($reply) > 0) {
	echo 'REPLY> ' . $reply . PHP_EOL;
	print_r(json_decode($reply, true));
	echo "Timing = " . (($endTime - $startTime)*1000) . ' ms' . PHP_EOL;
	echo PHP_EOL;
} else {
	$context = null;
	echo "ERROR.  Timeout occurred." . PHP_EOL;
}
