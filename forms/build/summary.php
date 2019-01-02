<?php

function tpps_summary_create_form(&$form, $form_state){
    
    $supplemental_upload_location = 'public://' . variable_get('tpps_supplemental_files_dir', 'tpps_supplemental');
    
    $form['comments'] = array(
      '#type' => 'textarea',
      '#title' => 'If you have any additional comments about this submission you would like to include, please write them here:',
      '#prefix' => tpps_table_display($form_state),
    );
    
    $form['files'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#title' => t('<div class="fieldset-title">Additional Files</div>'),
      '#description' => 'If there are any additional files you would like to include with your submission, please upload up to 10 files here.',
      '#collapsible' => TRUE
    );

    $form['files']['add'] = array(
      '#type' => 'button',
      '#title' => t('Add File'),
      '#button_type' => 'button',
      '#value' => t('Add File')
    );

    $form['files']['remove'] = array(
      '#type' => 'button',
      '#title' => t('Remove File'),
      '#button_type' => 'button',
      '#value' => t('Remove File')
    );

    $form['files']['number'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($form_state['saved_values']['summarypage']['files']['number']) ? $form_state['saved_values']['summarypage']['files']['number'] : '0',
    );

    for($i = 1; $i <= 10; $i++){

        $form['files']["$i"] = array(
          '#type' => 'managed_file',
          '#title' => t("Supplemental File $i"),
          '#upload_validators' => array(
            // These were all the relevant file types I could think of.
            'file_validate_extensions' => array('csv tsv xlsx txt pdf vcf doc docx xls ppt pptx fa fasta img png jpeg jpg zip gz fsa_nt html flat fsa ai ')
          ),
          '#upload_location' => "$supplemental_upload_location",
        );
    }
    
    $form['Back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
    );
    
    $form['Next'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );
    
    return $form;
}
