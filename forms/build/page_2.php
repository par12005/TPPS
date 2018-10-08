<?php

require_once 'page_2_ajax.php';
require_once 'page_2_helper.php';

function page_2_create_form(&$form, $form_state){
    
    if (isset($form_state['saved_values']['secondPage'])){
        $values = $form_state['saved_values']['secondPage'];
    }
    else{
        $values = array();
    }
    
    studyDate('Starting', $form, $values, $form_state);
    
    studyDate('Ending', $form, $values, $form_state);
    
    studyLocation($form, $values, $form_state);
    
    $form['dataType'] = array(
      '#type' => 'select',
      '#title' => t('Data Type: *'),
      '#options' => array(
        0 => '- Select -',
        'Genotype' => 'Genotype',
        'Phenotype' => 'Phenotype',
        'Environment' => 'Environment',
        'Genotype x Phenotype' => 'Genotype x Phenotype',
        'Genotype x Environment' => 'Genotype x Environment',
        'Phenotype x Environment' => 'Phenotype x Environment',
        'Genotype x Phenotype x Environment' => 'Genotype x Phenotype x Environment',
      ),
    );

    $form['studyType'] = array(
      '#type' => 'select',
      '#title' => t('Study Type: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Natural Population (Landscape)',
        2 => 'Growth Chamber',
        3 => 'Greenhouse',
        4 => 'Experimental/Common Garden',
        5 => 'Plantation',
      ),
    );
    
    naturalPopulation($form, $values);
    
    growthChamber($form, $values);
    
    greenhouse($form, $values);
    
    commonGarden($form, $values);
    
    plantation($form, $values);
    
    $form['Back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
      '#prefix' => '<div class="input-description">* : Required Field</div>',
    );
    
    $form['Save'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
    
    $form['Next'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
    );

    return $form;
}
