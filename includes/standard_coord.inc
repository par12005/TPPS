<?php

function tpps_standard_coord($raw_coordinate){
    
    $matches = array();
    $valid = TRUE;

    if (preg_match('/(([0-9]+)° ?([0-9]+)\' ?([0-9|\.]+)" ?([N|S])),? ?(([0-9]+)° ?([0-9]+)\' ?([0-9|\.]+)" ?([E|W]))/', $raw_coordinate, $matches)){
        $lat = $matches[2] + ($matches[3] / 60) + ($matches[4] / 3600);
        $long = $matches[7] + ($matches[8] / 60) + ($matches[9] / 3600);
    }
    elseif (preg_match('/(([0-9]+)° ?([0-9|\.]+)\' ?([N|S])),? ?(([0-9]+)° ?([0-9|\.]+)\' ?([E|W]))/', $raw_coordinate, $matches)){
        $lat = $matches[2] + ($matches[3] / 60);
        $long = $matches[6] + ($matches[7] / 60);
    }
    elseif (preg_match('/([0-9|\.]+), ?[\+|-]?([0-9|\.]+)/', $raw_coordinate, $matches)){
        $lat = $matches[1];
        $long = $matches[2];
    }
    elseif (preg_match('/([0-9|\.]+)°? ?([N|S]),? ?([0-9|\.]+)°? ?([E|W])/', $raw_coordinate, $matches)){
        $lat = $matches[1];
        $long = $matches[3];
    }
    else {
        $valid = FALSE;
    }

    if (preg_match('/(-).+,/', $raw_coordinate, $matches) and $valid){
        $lat = $lat * -1;
    }
    if (preg_match('/,.*(-)/', $raw_coordinate, $matches) and $valid){
        $long = $long * -1;
    }

    if (preg_match('/([N|S]).+([E|W])/', $raw_coordinate, $matches) and $valid){
        if ($matches[1] == 'S'){
            $lat = $lat * -1;
        }
        if ($matches[2] == 'W'){
            $long = $long * -1;
        }
    }
    
    if ($valid){
        return "$lat,$long";
    }
    else {
        return FALSE;
    }
}
