<?php

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

    if (!isset($phenotype_number) and isset($form_state['saved_values'][PAGE_4][$id]['phenotype']['number'])){
        $phenotype_number = $form_state['saved_values'][PAGE_4][$id]['phenotype']['number'];
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
      '#tree' => TRUE,
      '#prefix' => "<div id=\"environment-$id\">",
      '#suffix' => '</div>',
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
                    ->fields('c', array('title', 'group_id', 'layer_id'))
                    ->condition('c.group_id', $group_id);
                $layers_results = $layers_query->execute();
                while (($layer = $layers_results->fetchObject())){
                    $params_query = db_select ('cartogratree_fields', 'f')
                       ->fields('f', array('display_name', 'field_id'))
                       ->condition('f.layer_id', $layer->layer_id);
                    $params_results = $params_query->execute();
                    $params = array();
                    while (($param = $params_results->fetchObject())){
                       $params[$param->field_id] = $param->display_name;
                    }
                    $options[$layer->layer_id] = array(
                      'group_id' => $layer->group_id,
                      'group' => $group->group_name,
                      'title' => $layer->title, 
                      'params' => $params
                    );
                }
            }
        }
        
        $fields['use_layers'] = array(
          '#type' => 'checkbox',
          '#title' => 'I used environmental layers in my study that are indexed by CartograTree.',
          '#description' => 'If the layer you used is not in the list below, then the administrator for this site might not have enabled the layer group you used. Please contact them for more information.'
        );

        $fields['env_layers'] = array(
          '#type' => 'fieldset',
          '#title' => 'Cartogratree Environmental Layers: *',
          '#collapsible' => TRUE,
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[environment][use_layers]"]' => array('checked' => TRUE)
            )
          )
        );
        
        $fields['env_params'] = array(
          '#type' => 'fieldset',
          '#title' => 'CartograTree Environmental Layer Parameters: *',
          '#collapsible' => TRUE,
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[environment][use_layers]"]' => array('checked' => TRUE)
            )
          )
        );
        
        foreach ($options as $layer_id => $layer_info){
            $layer_title = $layer_info['title'];
            $layer_group = $layer_info['group'];
            $layer_params = $layer_info['params'];
            $fields['env_layers'][$layer_title] = array(
              '#type' => 'checkbox',
              '#title' => "<strong>$layer_title</strong> - $layer_group",
              '#return_value' => $layer_id,
            );
            
            if (!empty($layer_params)){
                $fields['env_params']["$layer_title"] = array(
                  '#type' => 'fieldset',
                  '#title' => "$layer_title Parameters",
                  '#description' => "Please select the parameters you used from the $layer_title layer.",
                  '#states' => array(
                    'visible' => array(
                      ':input[name="' . $id . '[environment][env_layers][' . $layer_title . ']"]' => array('checked' => TRUE)
                    )
                  )
                );
                
                foreach ($layer_params as $param_id => $param){
                    $fields['env_params']["$layer_title"][$param] = array(
                      '#type' => 'checkbox',
                      '#title' => $param,
                      '#return_value' => $param_id
                    );
                }
            }
        }
    }
    
    $fields['env_manual_check'] = array(
      '#type' => 'checkbox',
      '#title' => 'I have environmental data that I collected myself.',
    );
    
    if (isset($form_state['values'][$id]['environment']['number']) and $form_state['triggering_element']['#name'] == "Add Environment Data-$id"){
        $form_state['values'][$id]['environment']['number']++;
    }
    elseif (isset($form_state['values'][$id]['environment']['number']) and $form_state['triggering_element']['#name'] == "Remove Environment Data-$id" and $form_state['values'][$id]['environment']['number'] > 0){
        $form_state['values'][$id]['environment']['number']--;
    }
    $environment_number = isset($form_state['values'][$id]['environment']['number']) ? $form_state['values'][$id]['environment']['number'] : NULL;
    
    if (!isset($environment_number) and isset($form_state['saved_values'][PAGE_4][$id]['environment']['number'])){
        $environment_number = $form_state['saved_values'][PAGE_4][$id]['environment']['number'];
    }
    if (!isset($environment_number)){
        $environment_number = 1;
    }
    
    $fields['number'] = array(
      '#type' => 'hidden',
      '#value' => "$environment_number"
    );

    $fields['env_manual'] = array(
      '#type' => 'fieldset',
      '#title' => 'Custom Environmental Data:',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[environment][env_manual_check]"]' => array('checked' => TRUE)
        )
      ),
      '#collapsible' => TRUE,
    );
    
    $fields['env_manual']['add'] = array(
      '#type' => 'button',
      '#name' => t("Add Environment Data-$id"),
      '#button_type' => 'button',
      '#value' => t('Add Environment Data'),
      '#ajax' => array(
        'callback' => 'update_environment',
        'wrapper' => "environment-$id"
      ),
    );

    $fields['env_manual']['remove'] = array(
      '#type' => 'button',
      '#name' => t("Remove Environment Data-$id"),
      '#button_type' => 'button',
      '#value' => t('Remove Environment Data'),
      '#ajax' => array(
        'callback' => 'update_environment',
        'wrapper' => "environment-$id"
      ),
    );

    for ($i = 1; $i <= $environment_number; $i++){

        $fields['env_manual']["$i"] = array(
          '#type' => 'fieldset',
          '#tree' => TRUE,
        );
        
        $fields['env_manual']["$i"]['name'] = array(
          '#type' => 'textfield',
          '#title' => "Environmental Data $i Name: *",
          '#prefix' => "<label><b>Environment Data $i:</b></label>",
          '#description' => t("Please provide the name of Environmental Data $i. Some example environmental data names might include \"soil chemistry\", \"rainfall\", \"average temperature\", etc."),
        );
        
        $fields['env_manual']["$i"]['description'] = array(
          '#type' => 'textfield',
          '#title' => t("Environmental Data $i Description: *"),
          '#description' => t("Please provide a short description of Environmental Data $i."),
        );
        
        $fields['env_manual']["$i"]['units'] = array(
          '#type' => 'textfield',
          '#title' => t("Environmental Data $i Units: *"),
          '#description' => t("Please provide the units of Environmental Data $i."),
        );
        
        $fields['env_manual']["$i"]['value'] = array(
          '#type' => 'textfield',
          '#title' => t("Environmental Data $i Value: *"),
          '#description' => t("Please provide the value of Environmental Data $i."),
        );
    }
    
    return $fields;
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
        $form_state['saved_values'][PAGE_4][$id]['genotype']['BioProject-id'] = $form_state['values'][$id]['genotype']['BioProject-id'];
    }
    elseif (isset($form_state['saved_values'][PAGE_4][$id]['genotype']['BioProject-id']) and $form_state['saved_values'][PAGE_4][$id]['genotype']['BioProject-id'] != ''){
        $bio_id = $form_state['saved_values'][PAGE_4][$id]['genotype']['BioProject-id'];
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
