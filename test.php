<?php

/**
 * @file
 */

/**
 *
 */
function tpps_test_page($form, &$form_state) {

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => 'Next',
  );

  return $form;
}

/**
 *
 */
function tpps_test_page_validate($form, &$form_state) {

  form_set_error('Next', 'error');
  // For ($i = 0; $i < 2000; $i++){
  //    $file = file_load($i);
  //    if ($file){
  //      //dpm($i);
  //      //dpm($file->uri);
  //      if (preg_match('/tpps/', $file->filename)){
  //        dpm($i);
  //        dpm($file->filename);
  //        //file_delete($file);
  //      }
  //    }
  //  }
}

/**
 *
 */
function tpps_test_page_submit($form, &$form_state) {

}
