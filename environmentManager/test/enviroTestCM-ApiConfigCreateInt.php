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
$requester->connect(SocketCoordinatorSingleton::getInstance()->getSocketName('getConfigurationManagerAPI'));

// Build a request object
$o = new StdClass();
$o->messageType = 'configRequestCreate';
$o->sequenceNumber = 1000;
$o->authUsername = 'factory';
$o->authPassword = 'factory';
$o->name = 'ENV_AIR_COOL_MANUAL_SETPOINT';
$o->category = 'ENV';
$o->type = 'I';
$o->value = 10;
$o->writeLevel = 'factory';
$o->readLevel = 'public';
$o->description = 'Enables a static PWM setpoint, automatic control is disabled.';
$o->rackId = -99;
$o->slotId = -99;

$startTime = microtime(true);

$requester->send(json_encode($o));
$reply = $requester->recv();

$endTime = microtime(true);

if ($reply != false && strlen($reply) > 0) {
	echo 'REPLY> ' . $reply;
	print_r(json_decode($reply, true));
	echo "Timing = " . (($endTime - $startTime)*1000) . ' ms' . PHP_EOL;
	echo PHP_EOL;
} else {
	$context = null;
	echo "ERROR.  Timeout occurred." . PHP_EOL;
}
