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
          '#default_value' => isset($values[$id]['phenotype']['check']) ? $values[$id]['phenotype']['check'] : NULL,
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
              '#default_value' => isset($values[$id]['phenotype']['phenotypes-meta']["$i"]['name']) ? $values[$id]['phenotype']['phenotypes-meta']["$i"]['name'] : NULL,
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
              '#default_value' => isset($values[$id]['phenotype']['phenotypes-meta']["$i"]['attribute']) ? $values[$id]['phenotype']['phenotypes-meta']["$i"]['attribute'] : NULL,
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
              '#default_value' => isset($values[$id]['phenotype']['phenotypes-meta']["$i"]['description']) ? $values[$id]['phenotype']['phenotypes-meta']["$i"]['description'] : NULL,
              '#description' => t("Please provide a short description of Phenotype $i"),
            );
            
            $fields['phenotypes-meta']["$i"]['units'] = array(
              '#type' => 'textfield',
              '#title' => t("Phenotype $i Units: *"),
              '#autocomplete_path' => 'units/autocomplete',
              '#default_value' => isset($values[$id]['phenotype']['phenotypes-meta']["$i"]['units']) ? $values[$id]['phenotype']['phenotypes-meta']["$i"]['units'] : NULL,
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
              '#default_value' => isset($values[$id]['phenotype']['phenotypes-meta']["$i"]['struct-check']) ? $values[$id]['phenotype']['phenotypes-meta']["$i"]['struct-check'] : NULL,
            );
            
            $fields['phenotypes-meta']["$i"]['structure'] = array(
              '#type' => 'textfield',
              '#title' => t("Phenotype $i Structure: *"),
              '#autocomplete_path' => 'structure/autocomplete',
              '#default_value' => isset($values[$id]['phenotype']['phenotypes-meta']["$i"]['structure']) ? $values[$id]['phenotype']['phenotypes-meta']["$i"]['structure'] : NULL,
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
              '#default_value' => isset($values[$id]['phenotype']['phenotypes-meta']["$i"]['val-check']) ? $values[$id]['phenotype']['phenotypes-meta']["$i"]['val-check'] : NULL,
            );
            
            $fields['phenotypes-meta']["$i"]['min'] = array(
              '#type' => 'textfield',
              '#title' => t("Phenotype $i Minimum Value (type 1 for binary): *"),
              '#default_value' => isset($values[$id]['phenotype']['phenotypes-meta']["$i"]['min']) ? $values[$id]['phenotype']['phenotypes-meta']["$i"]['min'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][phenotypes-meta][' . $i . '][val-check]"]' => array('checked' => TRUE)
                )
              ),
            );
            
            $fields['phenotypes-meta']["$i"]['max'] = array(
              '#type' => 'textfield',
              '#title' => t("Phenotype $i Maximum Value (type 2 for binary): *"),
              '#default_value' => isset($values[$id]['phenotype']['phenotypes-meta']["$i"]['max']) ? $values[$id]['phenotype']['phenotypes-meta']["$i"]['max'] : NULL,
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
          '#default_value' => isset($values[$id]['phenotype']['metadata']) ? $values[$id]['phenotype']['metadata'] : NULL,
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE)
            )
          ),
          '#tree' => TRUE
        );

        $fields['no-header'] = array(
          '#type' => 'checkbox',
          '#title' => t('My file has no header row'),
          '#default_value' => isset($values[$id]['phenotype']['no-header']) ? $values[$id]['phenotype']['no-header'] : NULL,
          '#ajax' => array(
            'wrapper' => "header-$id-wrapper",
            'callback' => 'metadata_header_callback',
          ),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE)
            )
          ),
        );

        $fields['metadata']['columns'] = array(
          '#type' => 'fieldset',
          '#title' => t('<div class="fieldset-title">Define Data</div>'),
          '#states' => array(
            'invisible' => array(
              ':input[name="' . $id . '_phenotype_metadata_upload_button"]' => array('value' => 'Upload')
            )
          ),
          '#description' => 'Please define which columns hold the required data: Phenotype name',
          '#prefix' => "<div id=\"header-$id-wrapper\">",
          '#suffix' => '</div>',
          '#collapsible' => TRUE
        );

        $file = 0;
        if (isset($form_state['values']["$id"]['phenotype']['metadata']) and $form_state['values']["$id"]['phenotype']['metadata'] != 0){
            $file = $form_state['values']["$id"]['phenotype']['metadata'];
            $form_state['saved_values']['fourthPage']["$id"]['phenotype']['metadata'] = $form_state['values']["$id"]['phenotype']['metadata'];
        }
        elseif (isset($form_state['saved_values']['fourthPage']["$id"]['phenotype']['metadata']) and $form_state['saved_values']['fourthPage']["$id"]['phenotype']['metadata'] != 0){
            $file = $form_state['saved_values']['fourthPage']["$id"]['phenotype']['metadata'];
        }

        if ($file != 0){
            if (($file = file_load($file))){
                $file_name = $file->uri;
                
                //stop using the file so it can be deleted if the user clicks 'remove'
                file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));

                $location = drupal_realpath("$file_name");
                $content = parse_xlsx($location);
                $no_header = FALSE;

                if (isset($form_state['complete form']["$id"]['phenotype']['no-header']['#value']) and $form_state['complete form']["$id"]['phenotype']['no-header']['#value'] == 1){
                    tpps_content_no_header($content);
                    $no_header = TRUE;
                }
                elseif (!isset($form_state['complete form']["$id"]['phenotype']['no-header']['#value']) and isset($values["$id"]['phenotype']['no-header']) and $values["$id"]['phenotype']['no-header'] == 1){
                    tpps_content_no_header($content);
                    $no_header = TRUE;
                }
                
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

                $first = TRUE;

                foreach ($content['headers'] as $item){
                    $fields['metadata']['columns'][$item] = array(
                      '#type' => 'select',
                      '#title' => t($item),
                      '#options' => $column_options,
                      '#default_value' => isset($values["$id"]['phenotype']['metadata-columns'][$item]) ? $values["$id"]['phenotype']['metadata-columns'][$item] : 0,
                      '#prefix' => "<td>",
                      '#suffix' => "</td>",
                      '#attributes' => array(
                        'data-toggle' => array('tooltip'),
                        'data-placement' => array('left'),
                        'title' => array("Select the type of data the '$item' column holds")
                      )
                    );

                    if ($first){
                        $first = FALSE;
                        $fields['metadata']['columns'][$item]['#prefix'] = "<div style='overflow-x:auto'><table border='1'><tbody><tr>" . $fields['metadata']['columns'][$item]['#prefix'];
                    }

                    if ($no_header){
                        $fields['metadata']['columns'][$item]['#title'] = '';
                        $fields['metadata']['columns'][$item]['#attributes']['title'] = array("Select the type of data column $item holds");
                    }
                }

                // display sample data
                $display = "</tr>";
                for ($j = 0; $j < 3; $j++){
                    if (isset($content[$j])){
                        $display .= "<tr>";
                        foreach ($content['headers'] as $item){
                            $display .= "<th>{$content[$j][$item]}</th>";
                        }
                        $display .= "</tr>";
                    }
                }
                $display .= "</tbody></table></div>";

                $fields['metadata']['columns'][$item]['#suffix'] .= $display;
            }
        }
        
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
          '#default_value' => isset($values[$id]['genotype']['file-type']) ? $values[$id]['genotype']['file-type'] : NULL,
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
          '#default_value' => isset($values[$id]['genotype']['file']) ? $values[$id]['genotype']['file'] : NULL,
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
        
        $fields['no-header'] = array(
          '#type' => 'checkbox',
          '#title' => t('My file has no header row'),
          '#default_value' => isset($values[$id]['genotype']['no-header']) ? $values[$id]['genotype']['no-header'] : NULL,
          '#ajax' => array(
            'wrapper' => "genotype-header-$id-wrapper",
            'callback' => 'genotype_header_callback',
          ),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[genotype][file-type][Genotype Assay]"]' => array('checked' => true)
            )
          ),
        );

        $fields['file']['columns'] = array(
          '#type' => 'fieldset',
          '#title' => t('<div class="fieldset-title">Define Data</div>'),
          '#states' => array(
            'invisible' => array(
              ':input[name="' . $id . '_genotype_file_upload_button"]' => array('value' => 'Upload')
            )
          ),
          '#description' => 'Please define which columns hold the required data: Tree Identifier, SNP Data',
          '#prefix' => "<div id=\"genotype-header-$id-wrapper\">",
          '#suffix' => '</div>',
          '#collapsible' => TRUE
        );
        
        $file = 0;
        if (isset($form_state['values'][$id]['genotype']['file']) and $form_state['values'][$id]['genotype']['file'] != 0){
            $file = $form_state['values'][$id]['genotype']['file'];
            //dpm($file);
        }
        elseif (isset($form_state['saved_values']['fourthPage'][$id]['genotype']['file']) and $form_state['saved_values']['fourthPage'][$id]['genotype']['file'] != 0){
            $file = $form_state['saved_values']['fourthPage'][$id]['genotype']['file'];
        }
        
        if ($file != 0){
            if (($file = file_load($file))){
                $file_name = $file->uri;
                
                //stop using the file so it can be deleted if the user clicks 'remove'
                file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));

                $location = drupal_realpath("$file_name");
                $content = parse_xlsx($location);
                $no_header = FALSE;

                if (isset($form_state['complete form'][$id]['genotype']['no-header']['#value']) and $form_state['complete form'][$id]['genotype']['no-header']['#value'] == 1){
                    tpps_content_no_header($content);
                    $no_header = TRUE;
                }
                elseif (!isset($form_state['complete form'][$id]['genotype']['no-header']['#value']) and isset($values[$id]['genotype']['no-header']) and $values[$id]['genotype']['no-header'] == 1){
                    tpps_content_no_header($content);
                    $no_header = TRUE;
                }

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

                $first = TRUE;

                foreach ($content['headers'] as $item){
                    $fields['file']['columns'][$item] = array(
                      '#type' => 'select',
                      '#title' => t($item),
                      '#options' => $column_options,
                      '#default_value' => isset($values[$id]['genotype']['file-columns'][$item]) ? $values[$id]['genotype']['file-columns'][$item] : 0,
                      '#prefix' => "<td>",
                      '#suffix' => "</td>",
                      '#attributes' => array(
                        'data-toggle' => array('tooltip'),
                        'data-placement' => array('left'),
                        'title' => array("Select the type of data the '$item' column holds")
                      )
                    );
                    
                    if (!isset($values[$id]['genotype']['file-columns'][$item]) and isset($form_state['complete form'][$id]['genotype']['marker-type']['SNPs']['#value']) and $form_state['complete form'][$id]['genotype']['marker-type']['SNPs']['#value']){
                        //if Genotype Assay file and the value has not been set, set default value to 'SNP Data'
                        $fields['file']['columns'][$item]['#default_value'] = 2;
                    }

                    if ($first){
                        $first = FALSE;
                        $fields['file']['columns'][$item]['#prefix'] = "<div style='overflow-x:auto'><table border='1'><tbody><tr>" . $fields['file']['columns'][$item]['#prefix'];
                    }

                    if ($no_header){
                        $fields['file']['columns'][$item]['#title'] = '';
                        $fields['file']['columns'][$item]['#attributes']['title'] = array("Select the type of data column $item holds");
                    }
                }

                // display sample data
                $display = "</tr>";
                for ($i = 0; $i < 3; $i++){
                    if (isset($content[$i])){
                        $display .= "<tr>";
                        foreach ($content['headers'] as $item){
                            $display .= "<th>{$content[$i][$item]}</th>";
                        }
                        $display .= "</tr>";
                    }
                }
                $display .= "</tbody></table></div>";

                $fields['file']['columns'][$item]['#suffix'] .= $display;
            }
        }
        
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
          '#default_value' => isset($values[$id]['genotype']['assay-design']) ? $values[$id]['genotype']['assay-design'] : NULL,
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
          '#default_value' => isset($values[$id]['genotype']['vcf']) ? $values[$id]['genotype']['vcf'] : NULL,
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
    
    $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];
    $data_type = $form_state['saved_values']['secondPage']['dataType'];
    //dpm($data_type);
    for ($i = 1; $i <= $organism_number; $i++){
        
        $name = $form_state['saved_values']['Hellopage']['organism']["$i"];
        
        $form["organism-$i"] = array(
          '#type' => 'fieldset',
          '#title' => t("<div class=\"fieldset-title\">$name:</div>"),
          '#tree' => TRUE,
          //'#collapsible' => TRUE
        );

        if ($data_type == '1' or $data_type == '3' or $data_type == '4'){
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
              '#default_value' => isset($values["organism-$i"]['phenotype']['file']) ? $values["organism-$i"]['phenotype']['file'] : NULL,
              '#tree' => TRUE,
            );

            $form["organism-$i"]['phenotype']['file-no-header'] = array(
              '#type' => 'checkbox',
              '#title' => t('My file has no header row'),
              '#default_value' => isset($values["organism-$i"]['phenotype']['file-no-header']) ? $values["organism-$i"]['phenotype']['file-no-header'] : NULL,
              '#ajax' => array(
                'wrapper' => "phenotype-header-$i-wrapper",
                'callback' => 'phenotype_header_callback',
              ),
            );

            $form["organism-$i"]['phenotype']['file']['columns'] = array(
              '#type' => 'fieldset',
              '#title' => t('<div class="fieldset-title">Define Data</div>'),
              '#states' => array(
                'invisible' => array(
                  ':input[name="organism-' . $i . '_phenotype_file_upload_button"]' => array('value' => 'Upload')
                )
              ),
              '#description' => 'Please define which columns hold the required data: Tree Identifier, Phenotype name, and Value(s)',
              '#prefix' => "<div id=\"phenotype-header-$i-wrapper\">",
              '#suffix' => '</div>',
              '#collapsible' => TRUE
            );

            $file = 0;
            if (isset($form_state['values']["organism-$i"]['phenotype']['file']) and $form_state['values']["organism-$i"]['phenotype']['file'] != 0){
                $file = $form_state['values']["organism-$i"]['phenotype']['file'];
                $form_state['saved_values']['fourthPage']["organism-$i"]['phenotype']['file'] = $form_state['values']["organism-$i"]['phenotype']['file'];
            }
            elseif (isset($form_state['saved_values']['fourthPage']["organism-$i"]['phenotype']['file']) and $form_state['saved_values']['fourthPage']["organism-$i"]['phenotype']['file'] != 0){
                $file = $form_state['saved_values']['fourthPage']["organism-$i"]['phenotype']['file'];
            }
            
            if ($file != 0){
                if (($file = file_load($file))){
                    $file_name = $file->uri;
                    
                    //stop using the file so it can be deleted if the user clicks 'remove'
                    file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));

                    $location = drupal_realpath("$file_name");
                    $content = parse_xlsx($location);
                    $no_header = FALSE;

                    if (isset($form_state['complete form']["organism-$i"]['phenotype']['file-no-header']['#value']) and $form_state['complete form']["organism-$i"]['phenotype']['file-no-header']['#value'] == 1){
                        tpps_content_no_header($content);
                        $no_header = TRUE;
                    }
                    elseif (!isset($form_state['complete form']["organism-$i"]['phenotype']['file-no-header']['#value']) and isset($values["organism-$i"]['phenotype']['file-no-header']) and $values["organism-$i"]['phenotype']['file-no-header'] == 1){
                        tpps_content_no_header($content);
                        $no_header = TRUE;
                    }

                    $column_options = array(
                      'N/A',
                      'Tree Identifier',
                      'Phenotype Name/Identifier',
                      'Value(s)'
                    );

                    $first = TRUE;

                    foreach ($content['headers'] as $item){
                        $form["organism-$i"]['phenotype']['file']['columns'][$item] = array(
                          '#type' => 'select',
                          '#title' => t($item),
                          '#options' => $column_options,
                          '#default_value' => isset($values["organism-$i"]['phenotype']['file-columns'][$item]) ? $values["organism-$i"]['phenotype']['file-columns'][$item] : 0,
                          '#prefix' => "<td>",
                          '#suffix' => "</td>",
                          '#attributes' => array(
                            'data-toggle' => array('tooltip'),
                            'data-placement' => array('left'),
                            'title' => array("Select the type of data the '$item' column holds")
                          )
                        );

                        if ($first){
                            $first = FALSE;
                            $form["organism-$i"]['phenotype']['file']['columns'][$item]['#prefix'] = "<div style='overflow-x:auto'><table border='1'><tbody><tr>" . $form["organism-$i"]['phenotype']['file']['columns'][$item]['#prefix'];
                        }

                        if ($no_header){
                            $form["organism-$i"]['phenotype']['file']['columns'][$item]['#title'] = '';
                            $form["organism-$i"]['phenotype']['file']['columns'][$item]['#attributes']['title'] = array("Select the type of data column $item holds");
                        }
                    }

                    // display sample data
                    $display = "</tr>";
                    for ($j = 0; $j < 3; $j++){
                        if (isset($content[$j])){
                            $display .= "<tr>";
                            foreach ($content['headers'] as $item){
                                $display .= "<th>{$content[$j][$item]}</th>";
                            }
                            $display .= "</tr>";
                        }
                    }
                    $display .= "</tbody></table></div>";

                    $form["organism-$i"]['phenotype']['file']['columns'][$item]['#suffix'] .= $display;
                }
            }
        }
        
        if ($data_type == '1' or $data_type == '2' or $data_type == '3' or $data_type == '5'){
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

function ajax_bioproject_callback(&$form, $form_state){
    
    $ajax_id = $form_state['triggering_element']['#parents'][0];
    
    return $form[$ajax_id]['genotype']['assembly-auto'];
}

function metadata_header_callback($form, $form_state){
    return $form[$form_state['triggering_element']['#parents'][0]]['phenotype']['metadata']['columns'];
}

function update_phenotype($form, &$form_state){
    $id = $form_state['triggering_element']['#parents'][0];
    
    return $form[$id]['phenotype'];
}

function genotype_header_callback($form, $form_state){
    return $form[$form_state['triggering_element']['#parents'][0]]['genotype']['file']['columns'];
}

function phenotype_header_callback($form, $form_state){
    return $form[$form_state['triggering_element']['#parents'][0]]['phenotype']['file']['columns'];
}

function snps_file_callback($form, $form_state){
    $id = $form_state['triggering_element']['#parents'][0];
    
    return $form[$id]['genotype']['file'];
}

function page_4_ref(&$fields, &$form_state, $values, $id, $genotype_upload_location){
    global $user;
    $uid = $user->uid;

    $options = array(
      'key' => 'filename',
      'recurse' => FALSE
    );
    
    $genome_dir = variable_get('tpps_local_genome_dir', NULL);
    
    if ($genome_dir){
        $results = file_scan_directory($genome_dir, '/^([A-Z]|[a-z]){4}$/', $options);
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

    $ref_genome_arr = array();
    $ref_genome_arr[0] = '- Select -';

    $ref_genome_arr["url"] = 'I can provide a URL to the website of my reference file(s)';
    $ref_genome_arr["bio"] = 'I can provide a GenBank accession number (BioProject, WGS, TSA) and select assembly file(s) from a list';
    $ref_genome_arr["manual"] = 'I can upload my own reference genome file';
    $ref_genome_arr["manual2"] = 'I can upload my own reference transcriptome file';
    $ref_genome_arr["none"] = 'I am unable to provide a reference assembly';

    $fields['ref-genome'] = array(
      '#type' => 'select',
      '#title' => t('Reference Assembly used: *'),
      '#options' => $ref_genome_arr,
      '#default_value' => isset($values[$id]['genotype']['ref-genome']) ? $values[$id]['genotype']['ref-genome'] : 0,
    );

    $fields['BioProject-id'] = array(
      '#type' => 'textfield',
      '#title' => t('BioProject Accession Number: *'),
      '#default_value' => isset($values[$id]['genotype']['BioProject-id']) ? $values[$id]['genotype']['BioProject-id'] : NULL,
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
                  '#default_value' => isset($form_state['saved_values']['fourthPage']["$id"]['genotype']['assembly-auto']["$item"]) ? $form_state['saved_values']['fourthPage']["$id"]['genotype']['assembly-auto']["$item"] : NULL,
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
    
    $fasta['file']['file_remote']['#default_value'] = isset($values[$id]['genotype']['tripal_fasta']['file']['file_remote']) ? $values[$id]['genotype']['tripal_fasta']['file']['file_remote'] : NULL;
    $fasta['file']['file_upload_existing']['#default_value'] = isset($values[$id]['genotype']['tripal_fasta']['file']['file_upload_existing']) ? $values[$id]['genotype']['tripal_fasta']['file']['file_upload_existing'] : NULL;
    $fasta['analysis_id']['#default_value'] = isset($values[$id]['genotype']['tripal_fasta']['analysis_id']) ? $values[$id]['genotype']['tripal_fasta']['analysis_id'] : NULL;
    $fasta['seqtype']['#default_value'] = isset($values[$id]['genotype']['tripal_fasta']['seqtype']) ? $values[$id]['genotype']['tripal_fasta']['seqtype'] : NULL;
    
    $upload = array(
      '#type' => 'managed_file',
      '#title' => '',
      '#description' => 'Remember to click the "Upload" button below to send your file to the server.  This interface is capable of uploading very large files.  If you are disconnected you can return, reload the file and it will resume where it left off.  Once the file is uploaded the "Upload Progress" will indicate "Complete".  If the file is already present on the server then the status will quickly update to "Complete".',
      '#upload_validators' => array(
        'file_validate_extensions' => array(implode(' ', $class::$file_types))
      ),
      '#default_value' => isset($values[$id]['genotype']['tripal_fasta']['file']['file_upload']) ? $values[$id]['genotype']['tripal_fasta']['file']['file_upload'] : 0,
      '#upload_location' => $tripal_upload_location,
    );
    
    $fasta['file']['file_upload'] = $upload;
    $fasta['analysis_id']['#required'] = $fasta['seqtype']['#required'] = FALSE;
    /*
//    $fields['ref-genome-other'] = array(
//      '#type' => 'textfield',
//      '#title' => t('URL to Reference Genome:'),
//      '#default_value' => isset($values[$id]['genotype']['ref-genome-other']) ? $values[$id]['genotype']['ref-genome-other'] : NULL,
//      '#states' => array(
//        'visible' => array(
//          ':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'url'),
//        )
//      ),
//      '#attributes' => array(
//        'data-toggle' => array('tooltip'),
//        'data-placement' => array('left'),
//        'title' => array('This should be a link to a reference genome on NCBI')
//      )
//    );
    
    
    $fields['assembly-user'] = array(
      '#type' => 'managed_file',
      '#title' => t('Assembly File: please provide an assembly file in FASTA or Multi-FASTA format: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('fsa_nt')
      ),
      '#default_value' => isset($values[$id]['genotype']['assembly-user']) ? $values[$id]['genotype']['assembly-user'] : NULL,
      '#states' => array(
        'visible' => array(
          array(
            array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual')),
            'or',
            array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual2')),
          )
        )
      ),
      '#tree' => TRUE
    );
    
    $fields['assembly-user']['columns'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Define Data</div>'),
      '#description' => 'Please define which column holds the scaffold/chromosome identifier:',
      '#states' => array(
        'invisible' => array(
          ':input[name="' . $id . '_genotype_assembly-user_upload_button"]' => array('value' => 'Upload')
        )
      ),
      '#collapsible' => TRUE
    );
    
    $file = 0;
    if (isset($form_state['values'][$id]['genotype']['assembly-user']) and $form_state['values'][$id]['genotype']['assembly-user'] != 0){
        $file = $form_state['values'][$id]['genotype']['assembly-user'];
        $form_state['saved_values']['fourthPage'][$id]['genotype']['assembly-user'] = $form_state['values'][$id]['genotype']['assembly-user'];
    }
    elseif (isset($form_state['saved_values']['fourthPage'][$id]['genotype']['assembly-user']) and $form_state['saved_values']['fourthPage'][$id]['genotype']['assembly-user'] != 0){
        $file = $form_state['saved_values']['fourthPage'][$id]['genotype']['assembly-user'];
    }
    
    if ($file != 0){
        if (($file = file_load($file))){
            $content = fopen($file->uri, 'r');
            
            //stop using the file so it can be deleted if the user clicks 'remove'
            file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));

            $first = TRUE;

            $column_options = array(
              'N/A',
              'Scaffold/Chromosome',
            );
            $headers = array();

            for ($i = 0; $i < 3; $i++){
                $line = fgets($content);
                $line = explode(' ', $line);

                if ($first){
                    foreach ($line as $col => $name){
                        $headers[$col] = $col;
                        $col = preg_replace('/[^a-zA-Z0-9-_\.]/', '', $col);
                        $fields['assembly-user']['columns'][$col] = array(
                          '#type' => 'select',
                          '#title' => '',
                          '#options' => $column_options,
                          '#default_value' => isset($values[$id]['genotype']['assembly-user-columns'][$col]) ? $values[$id]['genotype']['assembly-user-columns'][$col] : 0,
                          '#prefix' => "<td>",
                          '#suffix' => "</td>",
                          '#attributes' => array(
                            'data-toggle' => array('tooltip'),
                            'data-placement' => array('left'),
                            'title' => array("Select if this column holds the scaffold/chromosome identifier")
                          )
                        );

                        if ($first){
                            $first = FALSE;
                            $fields['assembly-user']['columns'][$col]['#prefix'] = "<div style='overflow-x:auto'><table border='1'><tbody><tr>" . $fields['assembly-user']['columns'][$col]['#prefix'];
                        }
                    }
                    $display = "<tr>";
                }
                if ($line[0][0] == '>'){
                    $display .= "<tr>";
                    for ($j = 0; $j < count($headers); $j++){
                        $display .= "<th>{$line[$j]}</th>";
                    }
                    $display .= "</tr>";
                }
                elseif (!isset($line)){
                    break;
                }
                else{
                    $i--;
                }

            }
            $display .= "</tbody></table></div>";

            $fields['assembly-user']['columns'][$col]['#suffix'] .= $display;
        }
    }*/
    
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

    $fields['marker-type']['SNPs']['#default_value'] = isset($values[$id]['genotype']['marker-type']['SNPs']) ? $values[$id]['genotype']['marker-type']['SNPs'] : NULL;
    $fields['marker-type']['SSRs/cpSSRs']['#default_value'] = isset($values[$id]['genotype']['marker-type']['SSRs/cpSSRs']) ? $values[$id]['genotype']['marker-type']['SSRs/cpSSRs'] : NULL;
    $fields['marker-type']['Other']['#default_value'] = isset($values[$id]['genotype']['marker-type']['Other']) ? $values[$id]['genotype']['marker-type']['Other'] : NULL;

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
      '#default_value' => isset($values[$id]['genotype']['SNPs']['genotyping-design']) ? $values[$id]['genotype']['SNPs']['genotyping-design'] : 0,
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
      '#default_value' => isset($values[$id]['genotype']['SNPs']['GBS']) ? $values[$id]['genotype']['SNPs']['GBS'] : 0,
    );

    $fields['SNPs']['GBS-other'] = array(
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][SNPs][GBS]"]' => array('value' => '5'),
          ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '1')
        )
      ),
      '#default_value' => isset($values[$id]['genotype']['SNPs']['GBS-other']) ? $values[$id]['genotype']['SNPs']['GBS-other'] : NULL,
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
      '#default_value' => isset($values[$id]['genotype']['SNPs']['targeted-capture']) ? $values[$id]['genotype']['SNPs']['targeted-capture'] : 0,
    );

    $fields['SNPs']['targeted-capture-other'] = array(
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][SNPs][targeted-capture]"]' => array('value' => '2'),
          ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '2')
        )
      ),
      '#default_value' => isset($values[$id]['genotype']['SNPs']['targeted-capture-other']) ? $values[$id]['genotype']['SNPs']['targeted-capture-other'] : NULL,
    );
    
    $fields['SSRs/cpSSRs'] = array(
      '#type' => 'textfield',
      '#title' => t('Define SSRs/cpSSRs Type: *'),
      '#default_value' => isset($values[$id]['genotype']['SSRs/cpSSRs']) ? $values[$id]['genotype']['SSRs/cpSSRs'] : NULL,
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][marker-type][SSRs/cpSSRs]"]' => array('checked' => true)
        )
      )
    );

    $fields['other-marker'] = array(
      '#type' => 'textfield',
      '#title' => t('Define Other Marker Type: *'),
      '#default_value' => isset($values[$id]['genotype']['other-marker']) ? $values[$id]['genotype']['other-marker'] : NULL,
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
                $content = parse_xlsx($location);
                
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
        }
        
        if (form_get_errors() and !$form_state['rebuild']){
            $form_state['rebuild'] = TRUE;
            $new_form = drupal_rebuild_form('tpps_master', $form_state, $form);
            
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
