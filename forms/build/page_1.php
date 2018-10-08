<?php
require_once 'page_1_helper.php';
require_once 'page_1_ajax.php';

function page_1_create_form(&$form, $form_state){
    
    if (isset($form_state['saved_values']['Hellopage'])){
        $values = $form_state['saved_values']['Hellopage'];
    }
    else{
        $values = array();
    }
    
    user_info($form, $values);
    
    publication($form, $values, $form_state);
    
    organism($form, $values);
    
    $form['Save'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#prefix' => '<div class="input-description">* : Required Field</div>',
    );
    
    $form['Next'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
    );
    
    return $form;
}
