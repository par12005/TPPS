<?php
function page_4_create_form(&$form, $form_state){
    
    if (isset($form_state['saved_values']['fourthPage'])){
        $values = $form_state['saved_values']['fourthPage'];
    }
    else{
        $values = array();
    }
    
    function phenotype(&$form, $values, $id){
        
        $fields = array(
          '#type' => 'fieldset',
          '#title' => t('Phenotype Information:')
        );
        
        $phenotype_number = isset($values[$id]['phenotype']['number']) ? $values[$id]['phenotype']['number'] : 1;
        
        $fields['add'] = array(
          '#type' => 'button',
          '#title' => t('Add Phenotype'),
          '#button_type' => 'button',
          '#value' => t('Add Phenotype'),
        );
        
        $fields['remove'] = array(
          '#type' => 'button',
          '#title' => t('Remove Phenotype'),
          '#button_type' => 'button',
          '#value' => t('Remove Phenotype'),
        );
        
        $fields['number'] = array(
          '#type' => 'textfield',
          '#default_value' => $phenotype_number,
        );
        
        $structure_arr = array();
        $dev_arr = array();
	
        /*$results = db_select('chado.phenotype_structure_cvterm', 'phenotype_structure_cvterm')
            ->fields('phenotype_structure_cvterm', array('name', 'definition'))
            ->execute();
		
        
        foreach ($results as $row){
            array_push($structure_arr, "$row->name : $row->definition");
        }
        
        $results = db_select('chado.phenotype_cvterm', 'phenotype_cvterm')
            ->fields('phenotype_cvterm', array('name', 'definition'))
            ->execute();
        
		
        foreach ($results as $row){
            array_push($dev_arr, "$row->name : $row->definition");
        }*/
        
        for ($i = 1; $i <= 30; $i++){
            
            $fields["$i"] = array(
              '#type' => 'fieldset',
              '#title' => t("Phenotype $i:"),
            );
            
            $fields["$i"]['name'] = array(
              '#type' => 'textfield',
              '#title' => t("Phenotype $i Name:"),
              '#autocomplete_path' => 'phenotype/autocomplete',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['name']) ? $values[$id]['phenotype']["$i"]['name'] : NULL,
            );
            
            $fields["$i"]['environment-check'] = array(
              '#type' => 'checkbox',
              '#title' => t('This is environmental data about the study'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['environment-check']) ? $values[$id]['phenotype']["$i"]['environment-check'] : NULL,
            );
            
            $fields["$i"]['environment'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][environment-check]"]' => array('checked' => TRUE)
                )
              )
            );
            
            $fields["$i"]['environment']['description'] = array(
              '#type' => 'textarea',
              '#title' => t("Phenotype $i Description:"),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['environment']['description']) ? $values[$id]['phenotype']["$i"]['environment']['description'] : NULL,
            );
            
            $fields["$i"]['environment']['units'] = array(
              '#type' => 'select',
              '#title' => t("Phenotype $i Units:"),
              '#options' => array(
                0 => '- Select -',
                1 => 'mm', 
                2 => 'cm',
                3 => 'm', 
                4 => 'Degrees Celsius',
                5 => 'Degrees Fahrenheit',
                6 => 'Other'
              ),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['environment']['units']) ? $values[$id]['phenotype']["$i"]['environment']['units'] : NULL,
            );
            
            $fields["$i"]['environment']['units-other'] = array(
              '#type' => 'textfield',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['environment']['units-other']) ? $values[$id]['phenotype']["$i"]['environment']['units-other'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][environment][units]"]' => array('value' => '6')
                )
              )
            );
            
            $fields["$i"]['non-environment'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][environment-check]"]' => array('checked' => FALSE)
                )
              )
            );
            
            $fields["$i"]['non-environment']['type'] = array(
              '#type' => 'select',
              '#title' => t("Phenotype $i Type:"),
              '#options' => array(
                0 => '- Select -',
                1 => 'Binary',
                2 => 'Quantitative',
                3 => 'Qualitative'
              ),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['type']) ? $values[$id]['phenotype']["$i"]['non-environment']['type'] : 0,
            );
            
            $fields["$i"]['non-environment']['binary'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][non-environment][type]"]' => array('value' => '1')
                )
              )
            );
            
            $fields["$i"]['non-environment']['binary'][1] = array(
              '#type' => 'textfield',
              '#title' => t('Type 1:'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['binary'][1]) ? $values[$id]['phenotype']["$i"]['non-environment']['binary'][1] : NULL,
            );
            
            $fields["$i"]['non-environment']['binary'][2] = array(
              '#type' => 'textfield',
              '#title' => t('Type 2:'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['binary'][2]) ? $values[$id]['phenotype']["$i"]['non-environment']['binary'][2] : NULL,
            );
            
            $fields["$i"]['non-environment']['quantitative'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][non-environment][type]"]' => array('value' => '2')
                )
              )
            );
            
            $fields["$i"]['non-environment']['quantitative']['min'] = array(
              '#type' => 'textfield',
              '#title' => t('Minimum'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['quantitative']['min']) ? $values[$id]['phenotype']["$i"]['non-environment']['quantitative']['min'] : NULL,
            );
            
            $fields["$i"]['non-environment']['quantitative']['max'] = array(
              '#type' => 'textfield',
              '#title' => t('Maximum'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['quantitative']['max']) ? $values[$id]['phenotype']["$i"]['non-environment']['quantitative']['max'] : NULL,
            );
            
            $fields["$i"]['non-environment']['description'] = array(
              '#type' => 'textarea',
              '#title' => t("Phenotype $i Description:"),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['description']) ? $values[$id]['phenotype']["$i"]['non-environment']['description'] : NULL,
            );
            
            $fields["$i"]['non-environment']['units'] = array(
              '#type' => 'select',
              '#title' => t("Phenotype $i Units:"),
              '#options' => array(
                0 => '- Select -',
                1 => 'mm', 
                2 => 'cm',
                3 => 'm', 
                4 => 'Degrees Celsius',
                5 => 'Degrees Fahrenheit',
                6 => 'Other'
              ),
              '#states' => array(
                'invisible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][non-environment][type]"]' => array('value' => '3')
                )
              ),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['units']) ? $values[$id]['phenotype']["$i"]['non-environment']['units'] : NULL,
            );
            
            $fields["$i"]['non-environment']['units-other'] = array(
              '#type' => 'textfield',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['units-other']) ? $values[$id]['phenotype']["$i"]['non-environment']['units-other'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][non-environment][units]"]' => array('value' => '6')
                )
              )
            );
            
            $fields["$i"]['non-environment']['structure'] = array(
              '#type' => 'select',
              '#title' => t('Plant Structure:'),
              '#options' => $structure_arr,
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['structure']) ? $values[$id]['phenotype']["$i"]['non-environment']['structure'] : NULL,
            );
            
            $fields["$i"]['non-environment']['developmental'] = array(
              '#type' => 'select',
              '#title' => t('Plant Developmental Stage:'),
              '#options' => $dev_arr,
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['developmental']) ? $values[$id]['phenotype']["$i"]['non-environment']['developmental'] : NULL,
            );
        }
        
        $fields['check'] = array(
          '#type' => 'checkbox',
          '#title' => t('I have >30 Phenotypes'),
          '#default_value' => isset($values[$id]['phenotype']['check']) ? $values[$id]['phenotype']['check'] : NULL,
        );
        
        $fields['metadata'] = array(
          '#type' => 'managed_file',
          '#title' => t('Please upload a file containing metadata about all of your phenotypes'),
          '#upload_location' => 'public://',
          '#upload_validators' => array(
            'file_validate_extensions' => array('csv tsv xlsx')
          ),
          '#default_value' => isset($values[$id]['phenotype']['file']) ? $values[$id]['phenotype']['file'] : NULL,
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE)
            )
          )
        );

        
        return $fields;
    }
    
    function genotype(&$form, &$form_state, $values, $id){
        
        $fields = array(
          '#type' => 'fieldset',
          '#title' => t('Genotype Information:')
        );
        
        $options = array(
	  'key' => 'filename',
	  'recurse' => FALSE
	);
	
        $results = file_scan_directory("/linuxshare/projects/treegenes/tgwebprod_store/FTP/Genomes", '/^([A-Z]|[a-z]){4}$/', $options);
        //dpm($results);
        
        $ref_genome_arr = array();
        $ref_genome_arr[0] = '- Select -';
        
	foreach($results as $key=>$value){
            //dpm($key);
            $query = db_select('chado.organismprop', 'organismprop')
                ->fields('organismprop', array('organism_id'))
                ->condition('value', $key)
                ->execute()
                ->fetchAssoc();
            //dpm($query['organism_id']);
            $query = db_select('chado.organism', 'organism')
                ->fields('organism', array('genus', 'species'))
                ->condition('organism_id', $query['organism_id'])
                ->execute()
                ->fetchAssoc();
            //dpm($query['genus'] . " " . $query['species']);

            $versions = file_scan_directory("/linuxshare/projects/treegenes/tgwebprod_store/FTP/Genomes/$key", '/^v([0-9]|.)+$/', $options);
            //dpm($versions);
            foreach($versions as $item){
                $opt_string = $query['genus'] . " " . $query['species'] . " " . $item->filename;
                $ref_genome_arr[$opt_string] = $opt_string;
            }
	}
	$ref_genome_arr["Other"] = 'Other';
	
        $fields['ref-genome'] = array(
          '#type' => 'select',
          '#title' => t('Reference Genome used:'),
          '#options' => $ref_genome_arr,
          '#default_value' => isset($values[$id]['genotype']['ref-genome']) ? $values[$id]['genotype']['ref-genome'] : 0,
        );//*/

	$fields['ref-genome-other'] = array(
	  '#type' => 'textfield',
	  '#title' => t('URL to reference Genome:'),
	  '#default_value' => isset($values[$id]['genotype']['ref-genome-other']) ? $values[$id]['genotype']['ref-genome-other'] : NULL,
	  '#states' => array(
	    'visible' => array(
	      ':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'Other'),
	    )
	  )
	);
        
        $fields['marker-type'] = array(
          '#type' => 'checkboxes',
          '#title' => t('Marker Type (select all that apply):'),
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
          '#title' => t('SNPs Information:'),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[genotype][marker-type][SNPs]"]' => array('checked' => true)
            )
          )
        );
        
         $fields['SNPs']['genotyping-design'] = array(
          '#type' => 'select',
          '#title' => t('Define Genotyping Design:'),
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
          '#title' => t('GBS Type'),
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
          '#title' => t('Targeted Capture Type'),
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
        
        $fields['SNPs']['BioProject-id'] = array(
          '#type' => 'textfield',
          '#title' => t('BioProject Accession Number:'),
          '#default_value' => isset($values[$id]['genotype']['SNPs']['BioProject-id']) ? $values[$id]['genotype']['SNPs']['BioProject-id'] : NULL,
          '#ajax' => array(
            'callback' => 'ajax_bioproject_callback',
            'wrapper' => "$id-assembly-auto",
          ),
          '#states' => array(
            'invisible' => array(
              ':input[name="' . $id . '[genotype][SNPs][assembly-check]"]' => array('checked' => TRUE)
            )
          )
        );
        
        $fields['SNPs']['assembly-user'] = array(
          '#type' => 'managed_file',
          '#title' => t('Assembly Files: (WGS/TSA)'),
          '#upload_location' => 'public://',
          '#upload_validators' => array(
            'file_validate_extensions' => array('fsa_nt')
          ),
          '#default_value' => isset($values[$id]['genotype']['SNPs']['assembly-user']) ? $values[$id]['genotype']['SNPs']['assembly-user'] : NULL,
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[genotype][SNPs][assembly-check]"]' => array('checked' => TRUE)
            )
          )
        );
        
        if (isset($form_state['values'][$id]['genotype']['SNPs']['BioProject-id']) and strlen($form_state['values'][$id]['genotype']['SNPs']['BioProject-id']) > 5){
            
            $bio_id = substr($form_state['values']["$id"]['genotype']['SNPs']['BioProject-id'], 5);
    
            $options = array();
            $url = "https://eutils.ncbi.nlm.nih.gov/entrez/eutils/elink.fcgi?dbfrom=bioproject&db=nuccore&id=" . $bio_id;
            $response_xml_data = file_get_contents($url);
            $data = simplexml_load_string($response_xml_data)->children()->children()->LinkSetDb->children();

            foreach ($data->Link as $link){
                array_push($options, $link->Id->__tostring());
            }

            $fields['SNPs']['assembly-auto'] = array(
              '#type' => 'fieldset',
              '#title' => 'Select all that apply:',
              '#tree' => TRUE,
              '#states' => array(
                'invisible' => array(
                  ':input[name="' . $id . '[genotype][assembly-check]"]' => array('checked' => TRUE)
                )
              ),
              '#prefix' => "<div id='$id-assembly-auto'>",
              '#suffix' => '</div>',
              '#description' => 'If this list needs to be refreshed, please refresh the page and re-enter the BioProject ID.'
            );

            foreach ($options as $item){
                $fields['SNPs']['assembly-auto']["$item"] = array(
                  '#type' => 'checkbox',
                  '#title' => t("$item"),
                  '#default_value' => isset($form_state['saved_values']['fourthPage']["$id"]['genotype']['assembly-auto']["$item"]) ? $form_state['saved_values']['fourthPage']["$id"]['genotype']['assembly-auto']["$item"] : NULL,
                );
            }
        }
        else {
            $fields['SNPs']['assembly-auto'] = array(
              '#type' => 'fieldset',
              '#title' => t('Waiting for BioProject accession number...'),
              '#tree' => TRUE,
              '#prefix' => "<div id='$id-assembly-auto'>",
              '#suffix' => '</div>',
              '#states' => array(
                'invisible' => array(
                  ':input[name="' . $id . '[genotype][SNPs][assembly-check]"]' => array('checked' => TRUE)
                )
              )
            );
        }
        
        $fields['SNPs']['assembly-check'] = array(
          '#type' => 'checkbox',
          '#title' => t('My assembly file is not in this list'),
          '#default_value' => isset($values[$id]['genotype']['SNPs']['assembly-check']) ? $values[$id]['genotype']['SNPs']['assembly-check'] : NULL,
        );
        
        $fields['SSRs/cpSSRs'] = array(
          '#type' => 'textfield',
          '#title' => t('Define Type:'),
          '#default_value' => isset($values[$id]['genotype']['SSRs/cpSSRs']) ? $values[$id]['genotype']['SSRs/cpSSRs'] : NULL,
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[genotype][marker-type][SSRs/cpSSRs]"]' => array('checked' => true)
            )
          )
        );
        
        $fields['other-marker'] = array(
          '#type' => 'textfield',
          '#title' => t('Define Type:'),
          '#default_value' => isset($values[$id]['genotype']['other-marker']) ? $values[$id]['genotype']['other-marker'] : NULL,
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[genotype][marker-type][Other]"]' => array('checked' => true)
            )
          )
        );
        
        $fields['file'] = array(
          '#type' => 'managed_file',
          '#title' => t('Genotype File:'),
          '#upload_location' => 'public://',
          '#upload_validators' => array(
            'file_validate_extensions' => array('xlsx')
          ),
          '#default_value' => isset($values[$id]['genotype']['file']) ? $values[$id]['genotype']['file'] : NULL,
          '#tree' => TRUE
        );
        
        $fields['file']['columns'] = array(
          '#type' => 'fieldset',
          '#title' => t('Columns'),
          '#states' => array(
            'invisible' => array(
              ':input[name="' . $id . '_genotype_file_upload_button"]' => array('value' => 'Upload')
            )
          ),
          '#description' => 'Please define which columns hold the required data:'
        );
        
        $file = 0;
        if (isset($form_state['values'][$id]['genotype']['file']) and $form_state['values'][$id]['genotype']['file'] != 0){
            $file = $form_state['values'][$id]['genotype']['file'];
        }
        elseif (isset($form_state['saved_values']['fourthPage'][$id]['genotype']['file']) and $form_state['saved_values']['fourthPage'][$id]['genotype']['file'] != 0){
            $file = $form_state['saved_values']['fourthPage'][$id]['genotype']['file'];
        }
        
        if ($file != 0){
            $file = file_load($file);
            $file_name = explode('//', $file->uri);
            $file_name = $file_name[1];

            //vm
            //$location = "/var/www/html/Drupal/sites/default/files/$file_name";
            //dev site
            $location = "/var/www/Drupal/sites/default/files/$file_name";
            $content = parse_xlsx($location);

            $column_options = array(
              'N/A',
              'Tree Identifier',
            );

            $first = TRUE;

            foreach ($content['headers'] as $item){
                $fields['file']['columns'][$item] = array(
                  '#type' => 'select',
                  '#title' => t($item),
                  '#options' => $column_options,
                  '#default_value' => isset($values[$id]['genotype']['file-columns'][$item]) ? $values[$id]['genotype']['file-columns'][$item] : 0,
                  '#prefix' => "<td>",
                  '#suffix' => "</td>"
                );
                
                if ($first){
                    $first = FALSE;
                    $fields['file']['columns'][$item]['#prefix'] = "<div><table><tbody><tr>" . $fields['file']['columns'][$item]['#prefix'];
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
        
        return $fields;
    }
    
    $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];
    $data_type = $form_state['saved_values']['secondPage']['dataType'];
    //dpm($data_type);
    for ($i = 1; $i <= $organism_number; $i++){
        
        $name = $form_state['saved_values']['Hellopage']['organism']["$i"]['species'];
        
        $form["organism-$i"] = array(
          '#type' => 'fieldset',
          '#title' => t($name . ":"),
          '#tree' => TRUE,
        );


        
        if ($data_type == '1' or $data_type == '3' or $data_type == '4'){
            $form["organism-$i"]['phenotype'] = phenotype($form, $values, "organism-$i");
            
            $form["organism-$i"]['phenotype']['file'] = array(
              '#type' => 'managed_file',
              '#title' => t('Please upload a file containing all of your phenotypic data:'),
              '#upload_location' => 'public://',
              '#upload_validators' => array(
                'file_validate_extensions' => array('csv tsv xlsx')
              ),
              '#default_value' => isset($values["organism-$i"]['phenotype']['file']) ? $values["organism-$i"]['phenotype']['file'] : NULL,
              '#tree' => TRUE,
            );
            
            $form["organism-$i"]['phenotype']['file']['columns'] = array(
              '#type' => 'fieldset',
              '#title' => t('Columns'),
              '#states' => array(
                'invisible' => array(
                  ':input[name="organism-' . $i . '_phenotype_file_upload_button"]' => array('value' => 'Upload')
                )
              )
            );

            $file = 0;
            if (isset($form_state['values']["organism-$i"]['phenotype']['file']) and $form_state['values']["organism-$i"]['phenotype']['file'] != 0){
                $file = $form_state['values']["organism-$i"]['phenotype']['file'];
            }
            elseif (isset($form_state['saved_values']['fourthPage']["organism-$i"]['phenotype']['file']) and $form_state['saved_values']['fourthPage']["organism-$i"]['phenotype']['file'] != 0){
                $file = $form_state['saved_values']['fourthPage']["organism-$i"]['phenotype']['file'];
            }
            
            if ($file != 0){
                $file = file_load($file);
                $file_name = explode('//', $file->uri);
                $file_name = $file_name[1];

                //vm
                //$location = "/var/www/html/Drupal/sites/default/files/$file_name";
                //dev site
                $location = "/var/www/Drupal/sites/default/files/$file_name";
                $content = parse_xlsx($location);

                $required_columns = array(
                  'Tree Identifier',
                  'Phenotype Name/Identifier',
                  'Value(s)'
                );

                $options_arr = $content['headers'];
                $options_arr['- Select -'] = '- Select -';

                foreach ($required_columns as $req){
                    $form["organism-$i"]['phenotype']['file']['columns'][$req] = array(
                      '#type' => 'select',
                      '#title' => t($req),
                      '#options' => $options_arr,
                      '#default_value' => isset($values["organism-$i"]['phenotype']['file-columns'][$req]) ? $values["organism-$i"]['phenotype']['file-columns'][$req] : '- Select -',
                    );
                }

                // display sample data
                $display = "";
                $display .= "<div><table><tbody>";
                $display .= "<tr>";
                foreach ($content['headers'] as $item){
                    $display .= "<th>$item</th>";
                }
                $display .= "</tr>";
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

                $form["organism-$i"]['phenotype']['file']['columns']['#suffix'] = $display;

            }
        }
        
        if ($data_type == '1' or $data_type == '2' or $data_type == '3' or $data_type == '5'){
            $form["organism-$i"]['genotype'] = genotype($form, $form_state, $values, "organism-$i");
        }
    }
    
    $form['Back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
    );
    
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Review Information and Submit')
    );

    return $form;
}

function ajax_bioproject_callback(&$form, $form_state){
    
    $ajax_id = $form_state['triggering_element']['#parents'][0];
    
    return $form["$ajax_id"]['genotype']['SNPs']['assembly-auto'];
    
}

function page_4_validate_form(&$form, &$form_state){
    
    function validate_phenotype($phenotype, $id, $form, &$form_state){
        $phenotype_number = $phenotype['number'];
        $phenotype_check = $phenotype['check'];
        $phenotype_file = $phenotype['metadata'];
        $phenotype_content = $phenotype['file'];
        
        if ($phenotype_check == '1'){
            if ($phenotype_file == ''){
                form_set_error("$id][phenotype][file", "Phenotype File: field is required.");
            }
            else{
                //validate phenotype file
                $file = file(file_load($phenotype_file)->uri);
                $file_type = file_load($phenotype_file)->filemime;
            }
        }
        else{
            for($i = 1; $i <= $phenotype_number; $i++){
                $current_phenotype = $phenotype[$i];
                $name = $current_phenotype['name'];
                $environment_check = $current_phenotype['environment-check'];

                if ($name == ''){
                    form_set_error("$id][phenotype][$i][name", "Phenotype $i Name: field is required.");
                }

                if ($environment_check == '1'){
                    $description = $current_phenotype['environment']['description'];
                    $units = $current_phenotype['environment']['units'];

                    if ($description == ''){
                        form_set_error("$id][phenotype][$i][environment][description", "Phenotype $i Description: field is required.");
                    }

                    if ($units == ''){
                        form_set_error("$id][phenotype][$i][environment][units", "Phenotype $i Units: field is required.");
                    }
                }
                else{
                    $type = $current_phenotype['non-environment']['type'];
                    $binary_1 = $current_phenotype['non-environment']['binary'][1];
                    $binary_2 = $current_phenotype['non-environment']['binary'][2];
                    $min = $current_phenotype['non-environment']['quantitative']['min'];
                    $max = $current_phenotype['non-environment']['quantitative']['max'];
                    $description = $current_phenotype['non-environment']['description'];
                    $units = $current_phenotype['non-environment']['units'];
                    $structure = $current_phenotype['non-environment']['structure'];
                    $developmental = $current_phenotype['non-environment']['developmental'];

                    if ($type == '0'){
                        form_set_error("$id][phenotype][$i][non-environment][type", "Phenotype $i Type: field is required.");
                    }
                    elseif ($type == '1'){
                        if ($binary_1 == ''){
                            form_set_error("$id][phenotype][$i][non-environment][binary][1", "Phenotype $i Binary Type 1: field is required.");
                        }
                        if ($binary_2 == ''){
                            form_set_error("$id][phenotype][$i][non-environment][binary][2", "Phenotype $i Binary Type 2: field is required.");
                        }
                    }
                    elseif($type == '2'){
                        if ($min == ''){
                            form_set_error("$id][phenotype][$i][non-environment][quantitative][min", "Phenotype $i Minimum: field is required.");
                        }
                        if ($max == ''){
                            form_set_error("$id][phenotype][$i][non-environment][quantitative][max", "Phenotype $i Maximum: field is required.");
                        }
                    }

                    if ($description == ''){
                        form_set_error("$id][phenotype][$i][non-environment][description", "Phenotype $i Description: field is required.");
                    }

                    if ($units == '' and $type == '2'){
                        form_set_error("$id][phenotype][$i][non-environment][units", "Phenotype $i Units: field is required.");
                    }

                    if ($structure == '0'){
                        form_set_error("$id][phenotype][$i][non-environment][structure", "Phenotype $i Plant Structure: field is required.");
                    }

                    if ($developmental == '0'){
                        form_set_error("$id][phenotype][$i][non-environment][developmental", "Phenotype $i Developmental Stage: field is required.");
                    }
                }
            }
        }
        
        if ($phenotype_content == ''){
            form_set_error("$id][phenotype][file", "Phenotypes: field is required.");
        }
        else {
            $required_columns = array(
              'Tree Identifier',
              'Phenotype Name/Identifier',
              'Value(s)'
            );

            $form_state['values'][$id]['phenotype']['file-columns'] = array();

            foreach ($required_columns as $req){
                $form_state['values'][$id]['phenotype']['file-columns'][$req] = $form[$id]['phenotype']['file']['columns'][$req]['#value'];

                $col_val = $form_state['values'][$id]['phenotype']['file-columns'][$req];
                if ($col_val == '- Select -'){
                    form_set_error("$id][phenotype][file][columns][$req", "$req: please select the appropriate column.");
                }
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
        $bio_id = $snps['BioProject-id'];
        $assembly_user = $snps['assembly-user'];
        $assembly_check = $snps['assembly-check'];
        $ref_genome = $genotype['ref-genome'];
        
        if ($ref_genome === '0'){
            form_set_error("$id][genotype][ref-genome", "Reference Genome: field is required.");
        }
        elseif($ref_genome === 'Other' and $genotype['ref-genome-other'] === ''){
            form_set_error("$id][genotype][ref-genome-other", "Custom Reference Genome: field is required.");
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
                if ($targed_capture == '0'){
                    form_set_error("$id][genotype][SNPs][targeted-capture", "Targeted Capture: field is required.");
                }
                elseif ($targeted_capture == '2' and $snps['targeted-capture-other'] == ''){
                    form_set_error("$id][genotype][SNPs][targeted-capture-other", "Custom Targeted Capture: field is required.");
                }
            }
            
            if ($assembly_check != '0'){
                if ($assembly_user == ''){
                    form_set_error("$id][genotype][SNPs][assembly-user", 'Assembly file upload: field is required.');
                }
            }
            else{
                $assembly_auto = $snps['assembly-auto'];
                if ($bio_id == ''){
                    form_set_error("$id][genotype][SNPs][Bioproject-id", 'BioProject Id: field is required.');
                }
            
                $assembly_auto_check = '';
                
                foreach ($assembly_auto as $item){
                    $assembly_auto_check += $item;
                }
        
                if (preg_match('/^0*$/', $assembly_auto_check)){
                    form_set_error("$id][genotype][SNPs][assembly-auto", 'Assembly files: field is required.');
                }
            }
        }
        elseif ($ssrs != '0' and $genotype['SSRs/cpSSRs'] == ''){
            form_set_error("$id][genotype][SSRs/cpSSRs", "SSRs/cpSSRs: field is required.");
        }
        elseif ($other_marker != '0' and $genotype['other-marker'] == ''){
            form_set_error("$id][genotype][other-marker", "Other Genotype marker: field is required.");
        }
    
        if ($genotype_file == ''){
            form_set_error("$id][genotype][file", "Genotype file: field is required.");
        }
        else {
            $required_columns = array(
              '1' => 'Tree Identifier',
            );

            $form_state['values'][$id]['genotype']['file-columns'] = array();

            foreach ($form[$id]['genotype']['file']['columns'] as $req => $val){
                if ($req[0] != '#'){
                    $form_state['values'][$id]['genotype']['file-columns'][$req] = $form[$id]['genotype']['file']['columns'][$req]['#value'];

                    $col_val = $form_state['values'][$id]['genotype']['file-columns'][$req];
                    if ($col_val != '0'){
                        $required_columns[$col_val] = NULL;
                    }
                }
            }
            
            foreach ($required_columns as $item){
                if ($item != NULL){
                    form_set_error("$id][genotype][file][columns][$item", "Genotype file: Please specify a column that holds $item.");
                }
            }
        }
    }
    
    $form_values = $form_state['values'];
    $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];
    $data_type = $form_state['saved_values']['secondPage']['dataType'];
    
    for ($i = 1; $i <= $organism_number; $i++){
        $organism = $form_values["organism-$i"];
        
        if ($data_type == '1' or $data_type == '3' or $data_type == '4'){
            $phenotype = $organism['phenotype'];
            validate_phenotype($phenotype, "organism-$i", $form, $form_state);
        }
        
        if ($data_type == '1' or $data_type == '2' or $data_type == '3' or $data_type == '5'){
            $genotype = $organism['genotype'];
            validate_genotype($genotype, "organism-$i", $form, $form_state);
        }
    }
    
    /*if (empty(form_get_errors())){
        form_set_error('submit', 'validation success');
    }*/
}

function page_4_submit_form(&$form, &$form_state){
    $form_state['redirect'] = 'fourthPage';
}
