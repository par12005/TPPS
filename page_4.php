<?php
function page_4_create_form(&$form, &$form_state){
    if (isset($form_state['saved_values']['fourthPage'])){
        $values = $form_state['saved_values']['fourthPage'];
    }
    else{
        $values = array();
    }
    
    $genotype_upload_location = 'public://' . variable_get('tpps_genotype_files_dir', 'tpps_genotype');
    $phenotype_upload_location = 'public://' . variable_get('tpps_phenotype_files_dir', 'tpps_phenotype');
    
    $form['#tree'] = TRUE;
    
    $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];
    $data_type = $form_state['saved_values']['secondPage']['dataType'];
    for ($i = 1; $i <= $organism_number; $i++){
        
        $name = $form_state['saved_values']['Hellopage']['organism']["$i"];
        
        $form["organism-$i"] = array(
          '#type' => 'fieldset',
          '#title' => t("<div class=\"fieldset-title\">$name:</div>"),
          '#tree' => TRUE,
          //'#collapsible' => TRUE
        );

        if (preg_match('/P/', $data_type)){
            if ($i > 1){
                $form["organism-$i"]['phenotype-repeat-check'] = array(
                  '#type' => 'checkbox',
                  '#title' => "Phenotype information for $name is the same as phenotype information for {$form_state['saved_values']['Hellopage']['organism'][$i - 1]}.",
                  '#default_value' => isset($values["organism-$i"]['phenotype-repeat-check']) ? $values["organism-$i"]['phenotype-repeat-check'] : 1,
                );
            }
            
            $form["organism-$i"]['phenotype'] = phenotype($form, $values, "organism-$i", $form_state, $phenotype_upload_location);
            
            if ($i > 1){
                $form["organism-$i"]['phenotype']['#states'] = array(
                  'invisible' => array(
                    ":input[name=\"organism-$i\[phenotype-repeat-check]\"]" => array('checked' => TRUE)
                  )
                );
            }
            
            $form["organism-$i"]['phenotype']['file'] = array(
              '#type' => 'managed_file',
              '#title' => t('Phenotype file: Please upload a file containing columns for Tree Identifier, Phenotype Name, and value for all of your phenotypic data: *'),
              '#upload_location' => "$phenotype_upload_location",
              '#upload_validators' => array(
                'file_validate_extensions' => array('csv tsv xlsx')
              ),
              '#tree' => TRUE,
            );

            $form["organism-$i"]['phenotype']['file']['empty'] = array(
              '#default_value' => isset($values["organism-$i"]['phenotype']['file']['empty']) ? $values["organism-$i"]['phenotype']['file']['empty'] : 'NA'
            );

            $form["organism-$i"]['phenotype']['file']['columns'] = array(
              '#description' => 'Please define which columns hold the required data: Tree Identifier, Phenotype name, and Value(s)',
            );
            
            $column_options = array(
              'N/A',
              'Tree Identifier',
              'Phenotype Name/Identifier',
              'Value(s)'
            );
            
            $form["organism-$i"]['phenotype']['file']['columns-options'] = array(
              '#type' => 'hidden',
              '#value' => $column_options,
            );
            
            $form["organism-$i"]['phenotype']['file']['no-header'] = array();
        }
        
        if (preg_match('/G/', $data_type)){
            if ($i > 1){
                $form["organism-$i"]['genotype-repeat-check'] = array(
                  '#type' => 'checkbox',
                  '#title' => "Genotype information for $name is the same as genotype information for {$form_state['saved_values']['Hellopage']['organism'][$i - 1]}.",
                  '#default_value' => isset($values["organism-$i"]['genotype-repeat-check']) ? $values["organism-$i"]['genotype-repeat-check'] : 1,
                );
            }
            
            $form["organism-$i"]['genotype'] = genotype($form, $form_state, $values, "organism-$i", $genotype_upload_location);
            
            if ($i > 1){
                $form["organism-$i"]['genotype']['#states'] = array(
                  'invisible' => array(
                    ":input[name=\"organism-$i\[genotype-repeat-check]\"]" => array('checked' => TRUE)
                  )
                );
            }
            
        }
        
        if (preg_match('/E/', $data_type)){
            if ($i > 1){
                $form["organism-$i"]['environment-repeat-check'] = array(
                  '#type' => 'checkbox',
                  '#title' => "Environmental information for $name is the same as environmental information for {$form_state['saved_values']['Hellopage']['organism'][$i - 1]}.",
                  '#default_value' => isset($values["organism-$i"]['environment-repeat-check']) ? $values["organism-$i"]['environment-repeat-check'] : 1,
                );
            }
            
            $form["organism-$i"]['environment'] = environment($form, $form_state, $values, "organism-$i");
            
            if ($i > 1){
                $form["organism-$i"]['environment']['#states'] = array(
                  'invisible' => array(
                    ":input[name=\"organism-$i\[environment-repeat-check]\"]" => array('checked' => TRUE)
                  )
                );
            }
        }
    }
    
    $form['Back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
      '#prefix' => '<div class="input-description">* : Required Field</div>',
    );
    
    $form['Save'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
    
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Review Information and Submit')
    );
    
    return $form;
}

function phenotype(&$form, $values, $id, &$form_state, $phenotype_upload_location){

    $fields = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Phenotype Information:</div>'),
      '#tree' => TRUE,
      '#prefix' => "<div id=\"phenotypes-$id\">",
      '#suffix' => '</div>',
      '#description' => t('Upload a file and/or fill in form fields below to provide us with metadata about your phenotypes.'),
      '#collapsible' => TRUE,
    );

    if (isset($form_state['values'][$id]['phenotype']['number']) and $form_state['triggering_element']['#name'] == "Add Phenotype-$id"){
        $form_state['values'][$id]['phenotype']['number']++;
    }
    elseif (isset($form_state['values'][$id]['phenotype']['number']) and $form_state['triggering_element']['#name'] == "Remove Phenotype-$id" and $form_state['values'][$id]['phenotype']['number'] > 0){
        $form_state['values'][$id]['phenotype']['number']--;
    }
    $phenotype_number = isset($form_state['values'][$id]['phenotype']['number']) ? $form_state['values'][$id]['phenotype']['number'] : NULL;

    if (!isset($phenotype_number) and isset($form_state['saved_values']['fourthPage'][$id]['phenotype']['number'])){
        $phenotype_number = $form_state['saved_values']['fourthPage'][$id]['phenotype']['number'];
    }
    if (!isset($phenotype_number)){
        $phenotype_number = 0;
    }

    $fields['check'] = array(
      '#type' => 'checkbox',
      '#title' => t('I would like to upload a phenotype metadata file'),
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('left'),
        'title' => array('Upload a file')
      )
    );

    $fields['add'] = array(
      '#type' => 'button',
      '#name' => t("Add Phenotype-$id"),
      '#button_type' => 'button',
      '#value' => t('Add Phenotype'),
      '#ajax' => array(
        'callback' => 'update_phenotype',
        'wrapper' => "phenotypes-$id"
      ),
    );

    $fields['remove'] = array(
      '#type' => 'button',
      '#name' => t("Remove Phenotype-$id"),
      '#button_type' => 'button',
      '#value' => t('Remove Phenotype'),
      '#ajax' => array(
        'callback' => 'update_phenotype',
        'wrapper' => "phenotypes-$id"
      ),
    );

    $fields['number'] = array(
      '#type' => 'hidden',
      '#value' => "$phenotype_number"
    );

    $fields['phenotypes-meta'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );

    for ($i = 1; $i <= $phenotype_number; $i++){

        $fields['phenotypes-meta']["$i"] = array(
          '#type' => 'fieldset',
          '#tree' => TRUE,
        );

        $fields['phenotypes-meta']["$i"]['name'] = array(
          '#type' => 'textfield',
          '#title' => t("Phenotype $i Name: *"),
          '#autocomplete_path' => 'phenotype/autocomplete',
          '#prefix' => "<label><b>Phenotype $i:</b></label>",
          '#attributes' => array(
            'data-toggle' => array('tooltip'),
            'data-placement' => array('left'),
            'title' => array('If your phenotype name is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.')
          ),
          '#description' => t("Phenotype \"name\" is the human-readable name of the phenotype, where \"attribute\" is the thing that the phenotype is describing. Phenotype \"name\" should match the data in the \"Phenotype Name/Identifier\" column that you select in your <a href=\"#edit-$id-phenotype-file-ajax-wrapper\">Phenotype file</a> below.")
        );

        $fields['phenotypes-meta']["$i"]['attribute'] = array(
          '#type' => 'textfield',
          '#title' => t("Phenotype $i Attribute: *"),
          '#autocomplete_path' => 'attribute/autocomplete',
          '#attributes' => array(
            'data-toggle' => array('tooltip'),
            'data-placement' => array('left'),
            'title' => array('If your attribute is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.')
          ),
          '#description' => t('Some examples of attributes include: "amount", "width", "mass density", "area", "height", "age", "broken", "time", "color", "composition", etc.'),
        );

        $fields['phenotypes-meta']["$i"]['description'] = array(
          '#type' => 'textfield',
          '#title' => t("Phenotype $i Description: *"),
          '#description' => t("Please provide a short description of Phenotype $i"),
        );

        $fields['phenotypes-meta']["$i"]['units'] = array(
          '#type' => 'textfield',
          '#title' => t("Phenotype $i Units: *"),
          '#autocomplete_path' => 'units/autocomplete',
          '#attributes' => array(
            'data-toggle' => array('tooltip'),
            'data-placement' => array('left'),
            'title' => array('If your unit is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.')
          ),
          '#description' => t('Some examples of units include: "m", "meters", "in", "inches", "Degrees Celsius", "°C", etc.'),
        );

        $fields['phenotypes-meta']["$i"]['struct-check'] = array(
          '#type' => 'checkbox',
          '#title' => t("Phenotype $i has a structure descriptor"),
        );

        $fields['phenotypes-meta']["$i"]['structure'] = array(
          '#type' => 'textfield',
          '#title' => t("Phenotype $i Structure: *"),
          '#autocomplete_path' => 'structure/autocomplete',
          '#attributes' => array(
            'data-toggle' => array('tooltip'),
            'data-placement' => array('left'),
            'title' => array('If your structure is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.')
          ),
          '#description' => t('Some examples of structure descriptors include: "stem", "bud", "leaf", "xylem", "whole plant", "meristematic apical cell", etc.'),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][phenotypes-meta][' . $i . '][struct-check]"]' => array('checked' => TRUE)
            )
          ),
        );

        $fields['phenotypes-meta']["$i"]['val-check'] = array(
          '#type' => 'checkbox',
          '#title' => t("Phenotype $i has a value range"),
        );

        $fields['phenotypes-meta']["$i"]['min'] = array(
          '#type' => 'textfield',
          '#title' => t("Phenotype $i Minimum Value (type 1 for binary): *"),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][phenotypes-meta][' . $i . '][val-check]"]' => array('checked' => TRUE)
            )
          ),
        );

        $fields['phenotypes-meta']["$i"]['max'] = array(
          '#type' => 'textfield',
          '#title' => t("Phenotype $i Maximum Value (type 2 for binary): *"),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][phenotypes-meta][' . $i . '][val-check]"]' => array('checked' => TRUE)
            )
          ),
        );
    }

    $fields['metadata'] = array(
      '#type' => 'managed_file',
      '#title' => t('Phenotype Metadata File: Please upload a file containing columns with the name, attribute, description, and units of each of your phenotypes: *'),
      '#upload_location' => "$phenotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx')
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE)
        )
      ),
      '#tree' => TRUE
    );
    
    $fields['metadata']['empty'] = array(
      '#default_value' => isset($values["$id"]['phenotype']['metadata']['empty']) ? $values["$id"]['phenotype']['metadata']['empty'] : 'NA',
    );

    $fields['metadata']['columns'] = array(
      '#description' => 'Please define which columns hold the required data: Phenotype name',
    );
    
    $column_options = array(
      'N/A',
      'Phenotype Name/Identifier',
      'Attribute',
      'Description',
      'Units',
      'Structure',
      'Minimum Value',
      'Maximum Value'
    );
    
    $fields['metadata']['columns-options'] = array(
      '#type' => 'hidden',
      '#value' => $column_options,
    );
    
    $fields['metadata']['no-header'] = array();

    return $fields;
}

function genotype(&$form, &$form_state, $values, $id, $genotype_upload_location){

    $fields = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Genotype Information:</div>'),
      '#collapsible' => TRUE,
    );

    page_4_marker_info($fields, $values, $id);

    $fields['marker-type']['SNPs']['#ajax'] = array(
      'callback' => 'snps_file_callback',
      'wrapper' => "edit-$id-genotype-file-ajax-wrapper"
    );

    page_4_ref($fields, $form_state, $values, $id, $genotype_upload_location);

    $fields['file-type'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Genotype File Types (select all that apply): *'),
      '#options' => array(
        'Genotype Assay' => 'Genotype Spreadsheet/Assay',
        'Assay Design' => 'Assay Design',
        'VCF' => 'VCF',
      ),
    );
    
    $fields['file-type']['Assay Design']['#states'] = array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][marker-type][SNPs]"]' => array('checked' => TRUE),
      )
    );

    $fields['file'] = array(
      '#type' => 'managed_file',
      '#title' => t('Genotype Spreadsheet File: please provide a spreadsheet with columns for the Tree ID of genotypes used in this study: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('xlsx')
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][file-type][Genotype Assay]"]' => array('checked' => true)
        )
      ),
      '#description' => 0,
      '#tree' => TRUE
    );

    $assay_desc = "Please upload a spreadsheet file containing Genotype Assay data. When your file is uploaded, you will be shown a table with your column header names, several drop-downs, and the first few rows of your file. You will be asked to define the data type for each column, using the drop-downs provided to you. If a column data type does not fit any of the options in the drop-down menu, you may set that drop-down menu to \"N/A\". Your file must contain one column with the Tree Identifier, and one column for each SNP data associated with the study. Column data types will default to \"SNP Data\", so please leave any columns with SNP data as the default.";
    $spreadsheet_desc = "Please upload a spreadsheet file containing Genotype data. When your file is uploaded, you will be shown a table with your column header names, several drop-downs, and the first few rows of your file. You will be asked to define the data type for each column, using the drop-downs provided to you. If a column data type does not fit any of the options in the drop-down menu, you may set that drop-down menu to \"N/A\". Your file must contain one column with the Tree Identifier.";
    if (isset($form_state['complete form'][$id]['genotype']['marker-type']['SNPs']['#value']) and $form_state['complete form'][$id]['genotype']['marker-type']['SNPs']['#value']){
        $fields['file']['#description'] = $assay_desc;
    }
    if (!$fields['file']['#description'] and !isset($form_state['complete form'][$id]['genotype']['marker-type']['SNPs']['#value']) and isset($values[$id]['genotype']['marker-type']['SNPs']) and $values[$id]['genotype']['marker-type']['SNPs']){
        $fields['file']['#description'] = $assay_desc;
    }
    if (!$fields['file']['#description']){
        $fields['file']['#description'] = $spreadsheet_desc;
    }

    $fields['file']['empty'] = array(
      '#default_value' => isset($values[$id]['genotype']['file']['empty']) ? $values[$id]['genotype']['file']['empty'] : 'NA'
    );
    
    $fields['file']['columns'] = array(
      '#description' => 'Please define which columns hold the required data: Tree Identifier, SNP Data',
    );
    
    $column_options = array(
      'N/A',
      'Tree Identifier',
      'SNP Data',
    );
    
    if (isset($form_state['complete form'][$id]['genotype']['marker-type']['SNPs']['#value']) and !$form_state['complete form'][$id]['genotype']['marker-type']['SNPs']['#value']){
        $column_options[2] = 'Genotype Data';
    }
    elseif (!isset($form_state['complete form'][$id]['genotype']['marker-type']['SNPs']['#value']) and isset($values[$id]['genotype']['marker-type']['SNPs']) and !$values[$id]['genotype']['marker-type']['SNPs']){
        $column_options[2] = 'Genotype Data';
    }
    
    $fields['file']['columns-options'] = array(
      '#type' => 'hidden',
      '#value' => $column_options,
    );

    $fields['file']['no-header'] = array();
    
    $fields['assay-design'] = array(
      '#type' => 'managed_file',
      '#title' => 'Genotype Assay Design File: *',
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('xlsx')
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][file-type][Assay Design]"]' => array('checked' => TRUE),
          ':input[name="' . $id . '[genotype][marker-type][SNPs]"]' => array('checked' => TRUE),
        )
      ),
      '#tree' => TRUE,
    );

    if (isset($fields['assay-design']['#value'])){
        $fields['assay-design']['#default_value'] = $fields['assay-design']['#value'];
    }
    if (isset($fields['assay-design']['#default_value']) and $fields['assay-design']['#default_value'] and ($file = file_load($fields['assay-design']['#default_value']))){
        //stop using the file so it can be deleted if the user clicks 'remove'
        file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }

    $fields['vcf'] = array(
      '#type' => 'managed_file',
      '#title' => t('Genotype VCF File: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('vcf')
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][file-type][VCF]"]' => array('checked' => true)
        )
      ),
      '#tree' => TRUE
    );

    if (isset($fields['vcf']['#value'])){
        $fields['vcf']['#default_value'] = $fields['vcf']['#value'];
    }
    if (isset($fields['vcf']['#default_value']) and $fields['vcf']['#default_value'] and ($file = file_load($fields['vcf']['#default_value']))){
        //stop using the file so it can be deleted if the user clicks 'remove'
        file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }

    return $fields;
}

function environment(&$form, &$form_state, $values, $id){
    $cartogratree_env = variable_get('tpps_cartogratree_env', FALSE);
    
    $fields = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Environmental Information:</div>'),
      '#collapsible' => TRUE,
    );
    
    $fields['info'] = array(
      '#type' => 'textfield',
      '#title' => 'info',
    );
    
    if ($cartogratree_env){
        
        $query = db_select('variable', 'v')
            ->fields('v')
            ->condition('name', db_like('tpps_layer_group_') . '%', 'LIKE');
        
        $results = $query->execute();
        $options = array();
        
        while (($result = $results->fetchObject())){
            $group_id = substr($result->name, 17);
            $group = db_select('cartogratree_groups', 'g')
                ->fields('g', array('group_id', 'group_name'))
                ->condition('group_id', $group_id)
                ->execute()
                ->fetchObject();
            $group_is_enabled = variable_get("tpps_layer_group_$group_id", FALSE);
            
            if ($group_is_enabled){
                $layers_query = db_select('cartogratree_layers', 'c')
                    ->fields('c', array('title', 'group_id'))
                    ->condition('c.group_id', $group_id);
                $layers_results = $layers_query->execute();
                while (($layer = $layers_results->fetchObject())){
                    $options[$layer->title] = $group->group_name . ": <strong>" . $layer->title . '</strong>';
                }
            }
        }
        
        $fields['use_layers'] = array(
          '#type' => 'checkbox',
          '#title' => 'I used environmental layers in my study that are indexed by CartograTree.',
          '#description' => 'If the layer you used is not in the list below, then the administrator for this site might not have enabled the layer group you used. Please contact them for more information.'
        );

        $fields['env_layers'] = array(
          '#type' => 'checkboxes',
          '#title' => 'Cartogratree Environmental Layers:',
          '#options' => $options,
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[environment][use_layers]"]' => array('checked' => TRUE)
            )
          )
        );
        
        foreach ($options as $layer => $val){
            $fields['env_layers'][$layer]['#default_value'] = isset($values[$id]['environment']['env_layers'][$layer]) ? $values[$id]['environment']['env_layers'][$layer] : 0;
        }
    }
    
    return $fields;
}

function ajax_bioproject_callback(&$form, $form_state){
    
    $ajax_id = $form_state['triggering_element']['#parents'][0];
    
    return $form[$ajax_id]['genotype']['assembly-auto'];
}

function update_phenotype($form, &$form_state){
    $id = $form_state['triggering_element']['#parents'][0];
    
    return $form[$id]['phenotype'];
}

function snps_file_callback($form, $form_state){
    $id = $form_state['triggering_element']['#parents'][0];
    $commands = array();
    $commands[] = ajax_command_replace("#edit-$id-genotype-file-ajax-wrapper", drupal_render($form[$id]['genotype']['file']));
    if (!$form_state['complete form'][$id]['genotype']['file-type']['Genotype Assay']['#value']){
        $commands[] = ajax_command_invoke(".form-item-$id-genotype-file", 'hide');
    }
    else {
        $commands[] = ajax_command_invoke(".form-item-$id-genotype-file", 'show');
    }
    
    return array('#type' => 'ajax', '#commands' => $commands);
}

function page_4_ref(&$fields, &$form_state, $values, $id, $genotype_upload_location){
    global $user;
    $uid = $user->uid;

    $options = array(
      'key' => 'filename',
      'recurse' => FALSE
    );
    
    $genome_dir = variable_get('tpps_local_genome_dir', NULL);
    $ref_genome_arr = array();
    $ref_genome_arr[0] = '- Select -';
    
    if ($genome_dir){
        $results = file_scan_directory($genome_dir, '/^([A-Z][a-z]{3})$/', $options);
        foreach($results as $key=>$value){
            $query = db_select('chado.organismprop', 'organismprop')
                ->fields('organismprop', array('organism_id'))
                ->condition('value', $key)
                ->execute()
                ->fetchAssoc();
            $query = db_select('chado.organism', 'organism')
                ->fields('organism', array('genus', 'species'))
                ->condition('organism_id', $query['organism_id'])
                ->execute()
                ->fetchAssoc();
            
            $versions = file_scan_directory("$genome_dir/$key", '/^v([0-9]|.)+$/', $options);
            foreach($versions as $item){
                $opt_string = $query['genus'] . " " . $query['species'] . " " . $item->filename;
                $ref_genome_arr[$opt_string] = $opt_string;
            }
        }
    }

    $ref_genome_arr["url"] = 'I can provide a URL to the website of my reference file(s)';
    $ref_genome_arr["bio"] = 'I can provide a GenBank accession number (BioProject, WGS, TSA) and select assembly file(s) from a list';
    $ref_genome_arr["manual"] = 'I can upload my own reference genome file';
    $ref_genome_arr["manual2"] = 'I can upload my own reference transcriptome file';
    $ref_genome_arr["none"] = 'I am unable to provide a reference assembly';

    $fields['ref-genome'] = array(
      '#type' => 'select',
      '#title' => t('Reference Assembly used: *'),
      '#options' => $ref_genome_arr,
    );

    $fields['BioProject-id'] = array(
      '#type' => 'textfield',
      '#title' => t('BioProject Accession Number: *'),
      '#ajax' => array(
        'callback' => 'ajax_bioproject_callback',
        'wrapper' => "$id-assembly-auto",
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'bio')
        )
      )
    );
    
    $fields['assembly-auto'] = array(
      '#type' => 'fieldset',
      '#title' => t('Waiting for BioProject accession number...'),
      '#tree' => TRUE,
      '#prefix' => "<div id='$id-assembly-auto'>",
      '#suffix' => '</div>',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'bio')
        )
      )
    );
    
    if (isset($form_state['values'][$id]['genotype']['BioProject-id']) and $form_state['values'][$id]['genotype']['BioProject-id'] != ''){
        $bio_id = $form_state['values']["$id"]['genotype']['BioProject-id'];
        $form_state['saved_values']['fourthPage'][$id]['genotype']['BioProject-id'] = $form_state['values'][$id]['genotype']['BioProject-id'];
    }
    elseif (isset($form_state['saved_values']['fourthPage'][$id]['genotype']['BioProject-id']) and $form_state['saved_values']['fourthPage'][$id]['genotype']['BioProject-id'] != ''){
        $bio_id = $form_state['saved_values']['fourthPage'][$id]['genotype']['BioProject-id'];
    }
    elseif (isset($form_state['complete form']['organism-1']['genotype']['BioProject-id']['#value']) and $form_state['complete form']['organism-1']['genotype']['BioProject-id']['#value'] != '') {
        $bio_id = $form_state['complete form']['organism-1']['genotype']['BioProject-id']['#value'];
    }
    
    if (isset($bio_id) and $bio_id != ''){
        
        if (strlen($bio_id) > 5){
            $bio_id = substr($bio_id, 5);
        }
        
        $options = array();
        $url = "https://eutils.ncbi.nlm.nih.gov/entrez/eutils/elink.fcgi?dbfrom=bioproject&db=nuccore&id=" . $bio_id;
        $response_xml_data = file_get_contents($url);
        $link_types = simplexml_load_string($response_xml_data)->children()->children()->LinkSetDb;
        
        if (preg_match('/<LinkSetDb>/', $response_xml_data)){
            
            foreach($link_types as $type_xml){
                $type = $type_xml->LinkName->__tostring();
                
                switch ($type){
                    case 'bioproject_nuccore_tsamaster':
                        $suffix = 'TSA';
                        break;
                    
                    case 'bioproject_nuccore_wgsmaster':
                        $suffix = 'WGS';
                        break;
                    
                    default:
                        continue 2;
                }
                
                foreach ($type_xml->Link as $link){
                    $options[$link->Id->__tostring()] = $suffix;
                }
            }
            
            $fields['assembly-auto']['#title'] = '<div class="fieldset-title">Select all that apply: *</div>';
            $fields['assembly-auto']['#collapsible'] = TRUE;
            
            foreach ($options as $item => $suffix){
                $fields['assembly-auto']["$item"] = array(
                  '#type' => 'checkbox',
                  '#title' => "$item ($suffix) <a href=\"https://www.ncbi.nlm.nih.gov/nuccore/$item\" target=\"blank\">View on NCBI</a>",
                );
            }
        }
        else {
            $fields['assembly-auto']['#description'] = t('We could not find any assembly files related to that BioProject. Please ensure your accession number is of the format "PRJNA#"');
        }
    }
    
    require_once drupal_get_path('module', 'tripal') . '/includes/tripal.importer.inc';
    $class = 'FASTAImporter';
    tripal_load_include_importer_class($class);
    $tripal_upload_location = "public://tripal/users/$uid";
    
    $fasta = tripal_get_importer_form(array(), $form_state, $class);
    //dpm($fasta);
    
    $fasta['#type'] = 'fieldset';
    $fasta['#title'] = 'Tripal FASTA Loader';
    $fasta['#states'] = array(
      'visible' => array(
        array(
          array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'url')),
          'or',
          array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual')),
          'or',
          array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual2'))
        )
      )
    );
    
    unset($fasta['file']['file_local']);
    unset($fasta['organism_id']);
    unset($fasta['method']);
    unset($fasta['match_type']);
    $db = $fasta['additional']['db'];
    unset($fasta['additional']);
    $fasta['db'] = $db;
    $fasta['db']['#collapsible'] = TRUE;
    unset($fasta['button']);
    
    $upload = array(
      '#type' => 'managed_file',
      '#title' => '',
      '#description' => 'Remember to click the "Upload" button below to send your file to the server.  This interface is capable of uploading very large files.  If you are disconnected you can return, reload the file and it will resume where it left off.  Once the file is uploaded the "Upload Progress" will indicate "Complete".  If the file is already present on the server then the status will quickly update to "Complete".',
      '#upload_validators' => array(
        'file_validate_extensions' => array(implode(' ', $class::$file_types))
      ),
      '#upload_location' => $tripal_upload_location,
    );
    
    $fasta['file']['file_upload'] = $upload;
    $fasta['analysis_id']['#required'] = $fasta['seqtype']['#required'] = FALSE;
    
    $fields['tripal_fasta'] = $fasta;
    //dpm($fasta);
    
    return $fields;
}

function page_4_marker_info(&$fields, $values, $id){
    
    $fields['marker-type'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Marker Type (select all that apply): *'),
      '#options' => drupal_map_assoc(array(
        t('SNPs'),
        t('SSRs/cpSSRs'),
        t('Other'),
      ))
    );
    
    $fields['SNPs'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">SNPs Information:</div>'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][marker-type][SNPs]"]' => array('checked' => true)
        )
      ),
      '#collapsible' => TRUE
    );

     $fields['SNPs']['genotyping-design'] = array(
      '#type' => 'select',
      '#title' => t('Define Experimental Design: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'GBS',
        2 => 'Targeted Capture',
        3 => 'Whole Genome Resequencing',
        4 => 'RNA-Seq',
        5 => 'Genotyping Array'
      ),
    );

    $fields['SNPs']['GBS'] = array(
      '#type' => 'select',
      '#title' => t('GBS Type: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'RADSeq',
        2 => 'ddRAD-Seq',
        3 => 'NextRAD',
        4 => 'RAPTURE',
        5 => 'Other'
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '1')
        )
      ),
    );

    $fields['SNPs']['GBS-other'] = array(
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][SNPs][GBS]"]' => array('value' => '5'),
          ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '1')
        )
      ),
    );

    $fields['SNPs']['targeted-capture'] = array(
      '#type' => 'select',
      '#title' => t('Targeted Capture Type: *'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Exome Capture',
        2 => 'Other'
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '2')
        )
      ),
    );

    $fields['SNPs']['targeted-capture-other'] = array(
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][SNPs][targeted-capture]"]' => array('value' => '2'),
          ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '2')
        )
      ),
    );
    
    $fields['SSRs/cpSSRs'] = array(
      '#type' => 'textfield',
      '#title' => t('Define SSRs/cpSSRs Type: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][marker-type][SSRs/cpSSRs]"]' => array('checked' => true)
        )
      )
    );

    $fields['other-marker'] = array(
      '#type' => 'textfield',
      '#title' => t('Define Other Marker Type: *'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][marker-type][Other]"]' => array('checked' => true)
        )
      )
    );
    
    return $fields;
}

function page_4_validate_form(&$form, &$form_state){
    
    if ($form_state['submitted'] == '1'){
        
        function validate_phenotype($phenotype, $id, $form, &$form_state){
            $phenotype_number = $phenotype['number'];
            $phenotype_check = $phenotype['check'];
            $phenotype_meta = $phenotype['metadata'];
            $phenotype_file = $phenotype['file'];

            if ($phenotype_check == '1'){
                if ($phenotype_meta == ''){
                    form_set_error("$id][phenotype][metadata", "Phenotype Metadata File: field is required.");
                }
                else{
                    $required_groups = array(
                      'Phenotype Id' => array(
                        'id' => array(1),
                      ),
                      'Attribute' => array(
                        'attr' => array(2),
                      ),
                      'Description' => array(
                        'desc' => array(3),
                      ),
                      'Units' => array(
                        'units' => array(4),
                      )
                    );
                    
                    $file_element = $form[$id]['phenotype']['metadata'];
                    $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
                    
                    if (!form_get_errors()){
                        //get phenotype name column
                        $phenotype_name_col = $groups['Phenotype Id']['1'];
                        
                        //preserve file if it is valid
                        $file = file_load($form_state['values'][$id]['phenotype']['metadata']);
                        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
                    }
                }
            }
            
            for($i = 1; $i <= $phenotype_number; $i++){
                $current_phenotype = $phenotype['phenotypes-meta']["$i"];
                $name = $current_phenotype['name'];
                $attribute = $current_phenotype['attribute'];
                $description = $current_phenotype['description'];
                $units = $current_phenotype['units'];

                if ($name == ''){
                    form_set_error("$id][phenotype][phenotypes-meta][$i][name", "Phenotype $i Name: field is required.");
                }

                if ($attribute == ''){
                    form_set_error("$id][phenotype][phenotypes-meta][$i][attribute", "Phenotype $i Attribute: field is required.");
                }

                if ($description == ''){
                    form_set_error("$id][phenotype][phenotypes-meta][$i][description", "Phenotype $i Description: field is required.");
                }

                if ($units == ''){
                    form_set_error("$id][phenotype][phenotypes-meta][$i][units", "Phenotype $i Units: field is required.");
                }

                if ($current_phenotype['struct-check'] == '1' and $current_phenotype['structure'] == ''){
                    form_set_error("$id][phenotype][phenotypes-meta][$i][structure", "Phenotype $i Structure: field is required.");
                }

                if ($current_phenotype['val-check'] == '1' and $current_phenotype['min'] == ''){
                    form_set_error("$id][phenotype][phenotypes-meta][$i][min", "Phenotype $i Minimum Value: field is required.");
                }

                if ($current_phenotype['val-check'] == '1' and $current_phenotype['max'] == ''){
                    form_set_error("$id][phenotype][phenotypes-meta][$i][max", "Phenotype $i Maximum Value: field is required.");
                }
            }

            if ($phenotype_file == ''){
                form_set_error("$id][phenotype][file", "Phenotypes: field is required.");
            }
            else {
                $required_groups = array(
                  'Tree Identifier' => array(
                    'id' => array(1),
                  ),
                  'Phenotype Name/Identifier' => array(
                    'phenotype-name' => array(2),
                  ),
                  'Phenotype Value(s)' => array(
                    'val' => array(3),
                  )
                );
                
                $file_element = $form[$id]['phenotype']['file'];
                $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
                
                if (!form_get_errors()){
                    //get column names
                    $phenotype_file_tree_col = $groups['Tree Identifier']['1'];
                    $phenotype_file_name_col = $groups['Phenotype Name/Identifier']['2'];
                    
                    //preserve file if it is valid
                    $file = file_load($form_state['values'][$id]['phenotype']['file']);
                    file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
                }
            }
            
            if (empty(form_get_errors()) and isset($phenotype_file_name_col) and isset($phenotype_name_col)){
                $missing_phenotypes = tpps_compare_files($phenotype_file, $phenotype_meta, $phenotype_file_name_col, $phenotype_name_col);
                
                for ($i = 0; $i < count($missing_phenotypes); $i++){
                    $missing_phenotypes[$i];
                    for ($j = 1; $j <= $phenotype_number; $j++){
                        if (strtolower($phenotype['phenotypes-meta'][$j]['name']) == strtolower($missing_phenotypes[$i])){
                            unset($missing_phenotypes[$i]);
                            break;
                        }
                    }
                }
                
                if ($missing_phenotypes !== array()){
                    $phenotype_id_str = implode(', ', $missing_phenotypes);
                    form_set_error("$id][phenotype][file", "Phenotype file: We detected Phenotypes that were not in your Phenotype Metadata file. Please either remove these phenotypes from your Phenotype file, or add them to your Phenotype Metadata file. The phenotypes we detected with missing definitions were: $phenotype_id_str");
                }
            }
            elseif (empty(form_get_errors()) and isset($phenotype_file_name_col)){
                $phenotype_file = file_load($phenotype_file);
                $phenotype_file_name = $phenotype_file->uri;
                $location = drupal_realpath("$phenotype_file_name");
                $content = tpps_parse_xlsx($location);
                
                $missing_phenotypes = array();
                for ($i = 0; $i < count($content) - 1; $i++){
                    $used_phenotype = $content[$i][$phenotype_file_name_col];
                    $defined = FALSE;
                    for ($j = 1; $j <= $phenotype_number; $j++){
                        if (strtolower($phenotype['phenotypes-meta'][$j]['name']) == strtolower($used_phenotype)){
                            $defined = TRUE;
                            break;
                        }
                    }
                    if (!$defined){
                        array_push($missing_phenotypes, $used_phenotype);
                    }
                }
                
                if ($missing_phenotypes !== array()){
                    $phenotype_id_str = implode(', ', $missing_phenotypes);
                    form_set_error("$id][phenotype][file", "Phenotype file: We detected Phenotypes that were not in your Phenotype definitions. Please either remove these phenotypes from your Phenotype file, or add them to your Phenotype definitions. The phenotypes we detected with missing definitions were: $phenotype_id_str");
                }
            }
            
            if (empty(form_get_errors()) and isset($phenotype_file_tree_col)){
                
                if ($form_state['saved_values']['Hellopage']['organism']['number'] == 1 or $form_state['saved_values']['thirdPage']['tree-accession']['check'] == '0'){
                    $tree_accession_file = $form_state['saved_values']['thirdPage']['tree-accession']['file'];
                    $column_vals = $form_state['saved_values']['thirdPage']['tree-accession']['file-columns'];
                }
                else {
                    $num = substr($id, 9);
                    $tree_accession_file = $form_state['saved_values']['thirdPage']['tree-accession']["species-$num"]['file'];
                    $column_vals = $form_state['saved_values']['thirdPage']['tree-accession']["species-$num"]['file-columns'];
                }

                foreach ($column_vals as $col => $val){
                    if ($val == '1'){
                        $id_col_accession_name = $col;
                        break;
                    }
                }

                $missing_trees = tpps_compare_files($form_state['values'][$id]['phenotype']['file'], $tree_accession_file, $phenotype_file_tree_col, $id_col_accession_name);

                if ($missing_trees !== array()){
                    $tree_id_str = implode(', ', $missing_trees);
                    form_set_error("$id][phenotype][file", "Phenotype file: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Phenotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
                }
            }
        }

        function validate_genotype($genotype, $id, $form, &$form_state){
            $genotype_file = $genotype['file'];
            $marker_type = $genotype['marker-type'];
            $snps_check = $marker_type['SNPs'];
            $ssrs = $marker_type['SSRs/cpSSRs'];
            $other_marker = $marker_type['Other'];
            $marker_check = $snps_check . $ssrs . $other_marker;
            $snps = $genotype['SNPs'];
            $genotype_design = $snps['genotyping-design'];
            $gbs = $snps['GBS'];
            $targeted_capture = $snps['targeted-capture'];
            $bio_id = $genotype['BioProject-id'];
            $ref_genome = $genotype['ref-genome'];
            $file_type = $genotype['file-type'];
            $vcf = $genotype['vcf'];
            $assay_design = $genotype['assay-design'];
            
            if ($ref_genome === '0'){
                form_set_error("$id][genotype][ref-genome", "Reference Genome: field is required.");
            }
            elseif ($ref_genome === 'bio'){
                if ($bio_id == ''){
                    form_set_error("$id][genotype][Bioproject-id", 'BioProject Id: field is required.');
                }
                else {
                    $assembly_auto = $genotype['assembly-auto'];
                    $assembly_auto_check = '';
                    
                    foreach ($assembly_auto as $item){
                        $assembly_auto_check += $item;
                    }
                    
                    if (preg_match('/^0*$/', $assembly_auto_check)){
                        form_set_error("$id][genotype][assembly-auto", 'Assembly file(s): field is required.');
                    }
                }
            }
            elseif ($ref_genome === 'url' or $ref_genome === 'manual' or $ref_genome === 'manual2'){
                
                $class = 'FASTAImporter';
                tripal_load_include_importer_class($class);
                $fasta_vals = $genotype['tripal_fasta'];
                
                $file_upload = isset($fasta_vals['file']['file_upload']) ? trim($fasta_vals['file']['file_upload']) : 0;
                $file_existing = isset($fasta_vals['file']['file_upload_existing']) ? trim($fasta_vals['file']['file_upload_existing']) : 0;
                $file_remote = isset($fasta_vals['file']['file_remote']) ? trim($fasta_vals['file']['file_remote']) : 0;
                $db_id = trim($fasta_vals['db']['db_id']);
                $re_accession = trim($fasta_vals['db']['re_accession']);
                $analysis_id = trim($fasta_vals['analysis_id']);
                $seqtype = trim($fasta_vals['seqtype']);
                
                if (!$file_upload and !$file_existing and !$file_remote){
                    form_set_error("$id][genotype][tripal_fasta][file", "Assembly file: field is required.");
                }
                
                $re_name = '^(.*?)\s.*$';
                
                if ($db_id and !$re_accession){
                    form_set_error("$id][genotype][tripal_fasta][additional][re_accession", 'Accession regular expression: field is required.');
                }
                if ($re_accession and !$db_id){
                    form_set_error("$id][genotype][tripal_fasta][additional][db_id", 'External Database: field is required.');
                }
                
                if (!$analysis_id){
                    form_set_error("$id][genotype][tripal_fasta][analysis_id", 'Analysis: field is required.');
                }
                if (!$seqtype){
                    form_set_error("$id][genotype][tripal_fasta][seqtype", 'Sequence Type: field is required.');
                }
                
                //dpm($class::$file_required);
                //dpm($fasta_vals);
                //form_set_error("Submit", 'error');
                
                if (!form_get_errors()){
                    $assembly = $file_existing ? $file_existing : ($file_upload ? $file_upload : $file_remote);
                }
                
                /*$assembly = $genotype['assembly-user'];
                
                if ($assembly == ''){
                    form_set_error("$id][genotype][assembly-user", 'Assembly file: field is required.');
                }
                else {
                    $required_groups = array(
                      'Scaffold/Chromosome Id' => array(
                        'id' => array(1)
                      )
                    );
                    
                    $file_element = $form[$id]['genotype']['assembly-user'];
                    $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
                    
                    if (!form_get_errors()){
                        //get scaffold id column number
                        $scaffold_col = $groups['Scaffold/Chromosome Id']['1'];
                        
                        //preserve file if it is valid
                        $file = file_load($form_state['values'][$id]['genotype']['assembly-user']);
                        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
                    }
                }*/
            }
            
            if ($marker_check === '000'){
                form_set_error("$id][genotype][marker-type", "Genotype Marker Type: field is required.");
            }
            elseif($snps_check === 'SNPs'){
                if ($genotype_design == '0'){
                    form_set_error("$id][genotype][SNPs][genotyping-design", "Genotyping Design: field is required.");
                }
                elseif ($genotype_design == '1'){
                    if ($gbs == '0'){
                        form_set_error("$id][genotype][SNPs][GBS", "GBS Type: field is required.");
                    }
                    elseif ($gbs == '5' and $snps['GBS-other'] == ''){
                        form_set_error("$id][genotype][SNPs][GBS=other", "Custom GBS Type: field is required.");
                    }
                }
                elseif ($genotype_design == '2'){
                    if ($targeted_capture == '0'){
                        form_set_error("$id][genotype][SNPs][targeted-capture", "Targeted Capture: field is required.");
                    }
                    elseif ($targeted_capture == '2' and $snps['targeted-capture-other'] == ''){
                        form_set_error("$id][genotype][SNPs][targeted-capture-other", "Custom Targeted Capture: field is required.");
                    }
                }
            }
            elseif ($ssrs != '0' and $genotype['SSRs/cpSSRs'] == ''){
                form_set_error("$id][genotype][SSRs/cpSSRs", "SSRs/cpSSRs: field is required.");
            }
            elseif ($other_marker != '0' and $genotype['other-marker'] == ''){
                form_set_error("$id][genotype][other-marker", "Other Genotype marker: field is required.");
            }

            if ($file_type['VCF'] . $file_type['Genotype Assay'] === '00'){
                form_set_error("$id][genotype][file-type", "Genotype File Type: field is required.");
            }
            elseif ($file_type['VCF'] and $vcf == ''){
                form_set_error("$id][genotype][vcf", "Genotype VCF File: field is required.");
            }
            elseif ($file_type['VCF']){
                if (($ref_genome === 'manual' or $ref_genome === 'manual2' or $ref_genome === 'url') and isset($assembly) and $assembly and !form_get_errors()){
                    $vcf_content = fopen(file_load($vcf)->uri, 'r');
                    $assembly_content = fopen(file_load($assembly)->uri, 'r');
                    
                    while (($vcf_line = fgets($vcf_content)) !== FALSE){
                        if ($vcf_line[0] != '#'){
                            
                            $vcf_values = explode("\t", $vcf_line);
                            $scaffold_id = $vcf_values[0];
                            $match = FALSE;

                            while (($assembly_line = fgets($assembly_content)) !== FALSE){
                                if ($assembly_line[0] != '>'){
                                    continue;
                                }
                                else{
                                    if (preg_match('/^(.*?)\s.*$/', $assembly_line, $matches)){
                                        $assembly_scaffold = $matches[1];
                                    }
                                    if ($assembly_scaffold[0] == '>'){
                                        $assembly_scaffold = substr($assembly_scaffold, 1);
                                    }
                                    if ($assembly_scaffold == $scaffold_id){
                                        $match = TRUE;
                                        break;
                                    }
                                }
                            }
                            if (!$match){
                                fclose($assembly_content);
                                $assembly_content = fopen(file_load($assembly)->uri, 'r');
                                while (($assembly_line = fgets($assembly_content)) !== FALSE){
                                    if ($assembly_line[0] != '>'){
                                        continue;
                                    }
                                    else{
                                        if (preg_match('/^(.*?)\s.*$/', $assembly_line, $matches)){
                                            $assembly_scaffold = $matches[1];
                                        }
                                        if ($assembly_scaffold[0] == '>'){
                                            $assembly_scaffold = substr($assembly_scaffold, 1);
                                        }
                                        if ($assembly_scaffold == $scaffold_id){
                                            $match = TRUE;
                                            break;
                                        }
                                    }
                                }
                            }

                            if (!$match){
                                form_set_error('file', "VCF File: scaffold $scaffold_id not found in assembly file(s)");
                            }
                        }
                    }
                    
                }
                
                if (!form_get_errors()){
                    //preserve file if it is valid
                    $file = file_load($vcf);
                    file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
                }
            }
            
            if ($file_type['Genotype Assay'] and $genotype_file == ''){
                form_set_error("$id][genotype][file", "Genotype file: field is required.");
            }
            elseif ($file_type['Genotype Assay']) {
                
                $required_groups = array(
                  'Tree Id' => array(
                    'id' => array(1),
                  ),
                  'Genotype Data' => array(
                    'data' => array(2)
                  )
                );
                
                if ($snps_check){
                    unset($required_groups['Genotype Data']);
                    $required_groups['SNP Data'] = array(
                      'data' => array(2)
                    );
                }

                $file_element = $form[$id]['genotype']['file'];
                $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
                
                if (!form_get_errors()){
                    //get Tree Id column name
                    $id_col_genotype_name = $groups['Tree Id']['1'];
                    
                    if ($form_state['saved_values']['Hellopage']['organism']['number'] == 1 or $form_state['saved_values']['thirdPage']['tree-accession']['check'] == '0'){
                        $tree_accession_file = $form_state['saved_values']['thirdPage']['tree-accession']['file'];
                        $id_col_accession_name = $form_state['saved_values']['thirdPage']['tree-accession']['file-groups']['Tree Id']['1'];
                    }
                    else {
                        $num = substr($id, 9);
                        $tree_accession_file = $form_state['saved_values']['thirdPage']['tree-accession']["species-$num"]['file'];
                        $id_col_accession_name = $form_state['saved_values']['thirdPage']['tree-accession']["species-$num"]['file-groups']['Tree Id']['1'];
                    }
                    
                    $missing_trees = tpps_compare_files($form_state['values'][$id]['genotype']['file'], $tree_accession_file, $id_col_genotype_name, $id_col_accession_name);
                    
                    if ($missing_trees !== array()){
                        $tree_id_str = implode(', ', $missing_trees);
                        form_set_error("$id][genotype][file", "Genotype file: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Genotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
                    }
                }
                
                if (!form_get_errors()){
                    //preserve file if it is valid
                    $file = file_load($form_state['values'][$id]['genotype']['file']);
                    file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
                }
            }
            
            if ($file_type['Assay Design'] and $snps_check and $assay_design == ''){
                form_set_error("$id][genotype][assay-design", "Assay Design file: field is required.");
            }
            elseif ($file_type['Assay Design'] and $snps_check and !form_get_errors()){
                //preserve file if it is valid
                $file = file_load($form_state['values'][$id]['genotype']['assay-design']);
                file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
            }
        }
        
        function validate_environment($environment, $id, $form, &$form_state){
            if ($environment['use_layers']){
                //using cartogratree environment layers
                $layer_check = '';
                foreach ($environment['env_layers'] as $layer){
                    $layer_check .= $layer;
                }
                dpm($layer_check);
                if(preg_match('/^0+$/', $layer_check)){
                    form_set_error("$id][environment][env_layers]", 'CartograTree environmental layers: field is required.');
                }
            }
        }

        $form_values = $form_state['values'];
        $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];

        for ($i = 1; $i <= $organism_number; $i++){
            $organism = $form_values["organism-$i"];

            if ($i > 1 and isset($organism['phenotype-repeat-check']) and $organism['phenotype-repeat-check'] == '1'){
                unset($form_state['values']["organism-$i"]['phenotype']);
            }
            if (isset($form_state['values']["organism-$i"]['phenotype'])){
                $phenotype = $form_state['values']["organism-$i"]['phenotype'];
                validate_phenotype($phenotype, "organism-$i", $form, $form_state);
            }

            if ($i > 1 and isset($organism['genotype-repeat-check']) and $organism['genotype-repeat-check'] == '1'){
                unset($form_state['values']["organism-$i"]['genotype']);
            }
            if (isset($form_state['values']["organism-$i"]['genotype'])){
                $genotype = $form_state['values']["organism-$i"]['genotype'];
                validate_genotype($genotype, "organism-$i", $form, $form_state);
            }
            
            if ($i > 1 and isset($organism['environment-repeat-check']) and $organism['environment-repeat-check'] == '1'){
                unset($form_state['values']["organism-$i"]['environment']);
            }
            if (isset($form_state['values']["organism-$i"]['environment'])){
                $environment = $form_state['values']["organism-$i"]['environment'];
                validate_environment($environment, "organism-$i", $form, $form_state);
            }
        }
        
        if (form_get_errors() and !$form_state['rebuild']){
            $form_state['rebuild'] = TRUE;
            $new_form = drupal_rebuild_form('tpps_main', $form_state, $form);
            
            for ($i = 1; $i <= $organism_number; $i++){
                
                if (isset($new_form["organism-$i"]['phenotype']['metadata']['upload'])){
                    $form["organism-$i"]['phenotype']['metadata']['upload'] = $new_form["organism-$i"]['phenotype']['metadata']['upload'];
                    $form["organism-$i"]['phenotype']['metadata']['upload']['#id'] = "edit-organism-$i-phenotype-metadata-upload";
                }
                if (isset($new_form["organism-$i"]['phenotype']['metadata']['columns'])){
                    $form["organism-$i"]['phenotype']['metadata']['columns'] = $new_form["organism-$i"]['phenotype']['metadata']['columns'];
                    $form["organism-$i"]['phenotype']['metadata']['columns']['#id'] = "edit-organism-$i-phenotype-metadata-columns";
                }
                
                if (isset($form["organism-$i"]['phenotype']['file'])){
                    $form["organism-$i"]['phenotype']['file']['upload'] = $new_form["organism-$i"]['phenotype']['file']['upload'];
                    $form["organism-$i"]['phenotype']['file']['columns'] = $new_form["organism-$i"]['phenotype']['file']['columns'];
                    $form["organism-$i"]['phenotype']['file']['upload']['#id'] = "edit-organism-$i-phenotype-file-upload";
                    $form["organism-$i"]['phenotype']['file']['columns']['#id'] = "edit-organism-$i-phenotype-file-columns";
                }
                
                if (isset($form["organism-$i"]['genotype']['file']['upload']) and isset($new_form["organism-$i"]['genotype']['file']['upload'])){
                    $form["organism-$i"]['genotype']['file']['upload'] = $new_form["organism-$i"]['genotype']['file']['upload'];
                    $form["organism-$i"]['genotype']['file']['upload']['#id'] = "edit-organism-$i-genotype-file-upload";
                }
                if (isset($form["organism-$i"]['genotype']['file']['columns']) and isset($new_form["organism-$i"]['genotype']['file']['columns'])){
                    $form["organism-$i"]['genotype']['file']['columns'] = $new_form["organism-$i"]['genotype']['file']['columns'];
                    $form["organism-$i"]['genotype']['file']['columns']['#id'] = "edit-organism-$i-genotype-file-columns";
                }
//                
//                if (isset($form["organism-$i"]['genotype']['assembly-user']['upload'])){
//                    $form["organism-$i"]['genotype']['assembly-user']['upload'] = $new_form["organism-$i"]['genotype']['assembly-user']['upload'];
//                    $form["organism-$i"]['genotype']['assembly-user']['upload']['#id'] = "edit-organism-$i-genotype-assembly-user-upload";
//                }
//                if (isset($form["organism-$i"]['genotype']['assembly-user']['columns'])){
//                    $form["organism-$i"]['genotype']['assembly-user']['columns'] = $new_form["organism-$i"]['genotype']['assembly-user']['columns'];
//                    $form["organism-$i"]['genotype']['assembly-user']['columns']['#id'] = "edit-organism-$i-genotype-assembly-user-columns";
//                }
            }
        }
    }
    
    /*if (empty(form_get_errors())){
        form_set_error('submit', 'validation success');
    }*/
}

function page_4_submit_form(&$form, &$form_state){
    $form_state['redirect'] = 'fourthPage';
}
