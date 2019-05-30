<?php
/**
 * @file
 * Defines functions to create and populate Zenodo DOIs.
 */

function tpps_create_upload(){
  $ch = curl_init('https://zenodo.org/api/deposit/depositions?' . http_build_query(array('access_token' => TPPS_ZENODO_TOKEN)));
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json'));
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_setopt($ch, CURLOPT_POSTFIELDS, '{}');
  
  $out = json_decode(curl_exec($ch));
  //dpm(curl_getinfo($ch));
  curl_close($ch);
  dpm($out);
  return $out->id;
}

function tpps_upload_metadata($id, $metadata){
  
  $data = new stdClass();
  $data->metadata = $metadata;
  $data_json = json_encode($data);
  
  $ch = curl_init("https://zenodo.org/api/deposit/depositions/$id?" . http_build_query(array('access_token' => TPPS_ZENODO_TOKEN)));
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json', 'Content-Length: ' . strlen($data_json)));
  curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'PUT');
  curl_setopt($ch, CURLOPT_POSTFIELDS, $data_json);
  
  $out = json_decode(curl_exec($ch));
  curl_close($ch);
  dpm($out);
}
