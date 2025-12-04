#!/usr/bin/env php
<?php
declare(strict_types=1);
require_once(__DIR__ . '/../helpers/SocketCoordinatorSingleton.php');

$context = new ZMQContext();
$source = new ZMQSocket($context, ZMQ::SOCKET_PUSH);
$source->connect(SocketCoordinatorSingleton::getInstance()->getSocketName('getStatusUpdate'));

// Build a request object
$o = new StdClass();
$o->messageType = 'statusUpdate';
$o->fields = [];

$u = new StdClass();
$u->key = 'DRIVER_1_TEMP';
$u->label = 'DRIVER_1 TEMP';
$u->value = 25;
$u->units = 'C';
$u->timeoutMs = 30000;
array_push($o->fields, $u);

$u1 = new StdClass();
$u1->key = 'PA_1_TEMP';
$u1->label = 'PALLET_1 TEMP';
$u1->value = 30;
$u1->units = 'C';
$u1->timeoutMs = 30000;
array_push($o->fields, $u1);

$u2 = new StdClass();
$u2->key = 'PA_2_TEMP';
$u2->label = 'PALLET_2 TEMP';
$u2->value = 34;
$u2->units = 'C';
$u2->timeoutMs = 30000;
array_push($o->fields, $u2);

$u3 = new StdClass();
$u3->key = 'PA_3_TEMP';
$u3->label = 'PALLET_3 TEMP';
$u3->value = 35;
$u3->units = 'C';
$u3->timeoutMs = 30000;
array_push($o->fields, $u3);

$u4 = new StdClass();
$u4->key = 'PA_4_TEMP';
$u4->label = 'PALLET_4 TEMP';
$u4->value = 31;
$u4->units = 'C';
$u4->timeoutMs = 30000;
array_push($o->fields, $u4);

$testTemp = 0;
$dirDn = false;


// store values
$defaultTemp  = $u->value;
$defaultTemp1 = $u1->value;
$defaultTemp2 = $u2->value;
$defaultTemp3 = $u3->value;
$defaultTemp4 = $u4->value;


// $startTime = microtime(true);
while(true)
{
    $source->send(json_encode($o));

    // Modify on fly

    if($dirDn)
    {
        if(--$testTemp < -20)
        {
            $dirDn = false;
        }
    }
    else
    {
        if(++$testTemp > 50)
        {
            $dirDn = true;
        }
    }

    // modify values
    $u->value  = $defaultTemp  + $testTemp;
    $u1->value = $defaultTemp1 + $testTemp;
    $u2->value = $defaultTemp2 + $testTemp;
    $u3->value = $defaultTemp3 + $testTemp;
    $u4->value = $defaultTemp4 + $testTemp;

    sleep(5);
}

// $endTime = microtime(true);
