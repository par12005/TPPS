<?php

/**
 * 
 */

/**
 * 
 */
function tpps_api_submission($accession, $state = TRUE) {
  drupal_json_output(tpps_load_submission($accession, $state));
}