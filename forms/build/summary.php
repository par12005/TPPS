<?php

function tpps_summary_create_form(&$form, $form_state){
    
    $form['Back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
      '#prefix' => tpps_table_display($form_state),
    );
    
    $form['Next'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );
    
    return $form;
}
