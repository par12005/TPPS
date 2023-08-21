<?php

/**
 * @file
 * Creates GxPxE Data form page and includes helper files.
 */

require_once 'page_4_ajax.php';
require_once 'page_4_helper.php';

/**
 * Creates the GxPxE Data form page.
 *
 * This function creates the genotype, phenotype, and environmental fieldsets
 * based on the data type selection made on page 2. It will then call all
 * necessary helper functions.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 */
function tpps_page_4_create_form(array &$form, array &$form_state) {
  if (isset($form_state['saved_values'][TPPS_PAGE_4])) {
    $values = $form_state['saved_values'][TPPS_PAGE_4];
  }
  else {
    $values = array();
  }
  // dpm("FORM CREATED SAVED VALUES:");
  // dpm($values);

  $form['#tree'] = TRUE;

  $organism_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
  $data_type = $form_state['saved_values'][TPPS_PAGE_2]['data_type'];
  for ($i = 1; $i <= $organism_number; $i++) {

    $name = $form_state['saved_values'][TPPS_PAGE_1]['organism']["$i"]['name'];

    $form["organism-$i"] = array(
      '#type' => 'fieldset',
      '#title' => "<div class=\"fieldset-title\">$name:</div>",
      '#tree' => TRUE,
      '#collapsible' => TRUE,
    );

    if (preg_match('/P/', $data_type)) {
      if ($i > 1) {
        $form["organism-$i"]['phenotype-repeat-check'] = array(
          '#type' => 'checkbox',
          '#title' => "Phenotype information for $name is the same as phenotype information for {$form_state['saved_values'][TPPS_PAGE_1]['organism'][$i - 1]['name']}.",
          '#default_value' => isset($values["organism-$i"]['phenotype-repeat-check']) ? $values["organism-$i"]['phenotype-repeat-check'] : 1,
        );
      }

      $form["organism-$i"]['phenotype'] = tpps_phenotype($form, $form_state, $values, "organism-$i");

      if ($i > 1) {
        $form["organism-$i"]['phenotype']['#states'] = array(
          'invisible' => array(
            ":input[name=\"organism-$i\[phenotype-repeat-check]\"]" => array('checked' => TRUE),
          ),
        );
      }

      $normal_check = tpps_get_ajax_value($form_state, array(
        "organism-$i",
        'phenotype',
        'normal-check',
      ), TRUE);
      if (!empty($normal_check)) {
        $image_path = drupal_get_path('module', 'tpps') . '/images/';
        $form["organism-$i"]['phenotype']['format'] = array(
          '#type' => 'radios',
          '#title' => t('Phenotype file format: *'),
          '#options' => array(
            'Type 1',
            'Type 2',
          ),
          '#ajax' => array(
            'callback' => 'tpps_phenotype_file_format_callback',
            'wrapper' => "edit-organism-$i-phenotype-file-ajax-wrapper",
          ),
          '#default_value' => (isset($form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype']['format'])) ? $form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype']['format'] : 0,
          '#description' => t('Please select a file format type from the listed options. Below please see examples of each format type.'),
          '#states' => array(
            'invisible' => array(
              ":input[name=\"organism-{$i}[phenotype][phenotypes-meta][number]\"]" => array('value' => '0'),
              ":input[name=\"organism-{$i}[phenotype][check]\"]" => array('checked' => FALSE),
            ),
          ),
        );

        $form["organism-$i"]['phenotype']['format'][0]['#prefix'] = "<figure><img src=\"/{$image_path}phenotype_format_1.png\"><figcaption>";
        $form["organism-$i"]['phenotype']['format'][0]['#suffix'] = "</figcaption></figure>";
        $form["organism-$i"]['phenotype']['format'][1]['#prefix'] = "<figure><img src=\"/{$image_path}phenotype_format_2.png\"><figcaption>";
        $form["organism-$i"]['phenotype']['format'][1]['#suffix'] = "</figcaption></figure>";

        $form["organism-$i"]['phenotype']['file'] = array(
          '#type' => 'managed_file',
          '#title' => t('Phenotype file: Please upload a file containing columns for Plant Identifier, Phenotype Data: *'),
          '#upload_location' => 'public://' . variable_get('tpps_phenotype_files_dir', 'tpps_phenotype'),
          '#upload_validators' => array(
            'file_validate_extensions' => array('csv tsv'),
          ),
          '#tree' => TRUE,
          '#states' => array(
            'invisible' => array(
              ":input[name=\"organism-{$i}[phenotype][phenotypes-meta][number]\"]" => array('value' => '0'),
              ":input[name=\"organism-{$i}[phenotype][check]\"]" => array('checked' => FALSE),
            ),
          ),
        );

        $form["organism-$i"]['phenotype']['file']['empty'] = array(
          '#default_value' => isset($values["organism-$i"]['phenotype']['file']['empty']) ? $values["organism-$i"]['phenotype']['file']['empty'] : 'NA',
        );

        $form["organism-$i"]['phenotype']['file']['columns'] = array(
          '#description' => t('Please define which columns hold the required data: Plant Identifier, Phenotype name, and Value(s)'),
        );


        $format = tpps_get_ajax_value($form_state, array(
          "organism-$i",
          'phenotype',
          'format',
        ), 0);

        if ($format == 0) {
          $column_options = array(
            'Phenotype Data',
            'Plant Identifier',
            'Timepoint',
            'Clone Number',
            'N/A',
          );
        }
        else {
          $column_options = array(
            'N/A',
            'Plant Identifier',
            'Phenotype Name/Identifier',
            'Value(s)',
            'Timepoint',
            'Clone Number',
          );
          $form["organism-$i"]['phenotype']['file']['#title'] = t('Phenotype file: Please upload a file containing columns for Plant Identifier, Phenotype Name, and value for all of your phenotypic data: *');
        }

        $form["organism-$i"]['phenotype']['file']['columns-options'] = array(
          '#type' => 'hidden',
          '#value' => $column_options,
        );

        $form["organism-$i"]['phenotype']['file']['no-header'] = array();
      }

      // This will check if there are Time Phenotypes saved from the saved values
      // and use this to re-check the checkboxes on the form
      if (isset($form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype']['time']['time_phenotypes'])) {
        $count_time_phenotypes = count($form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype']['time']['time_phenotypes']);
        // dpm('TIME PHENOTYPES DETECTED: ' . $count_time_phenotypes);
        if($count_time_phenotypes > 0) {
          $time_phenotypes = $form_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype']['time']['time_phenotypes'];
          // dpm($time_phenotypes);
          // Go through all time phenotypes
          foreach ($time_phenotypes as $time_phenotype) {
            // dpm($time_phenotype);
            // if time_phenotype is not 0, meaning the phenotype name and phenotype value would be the same, so not 0
            if($time_phenotype != "0") {
              // dpm($time_phenotype);
              // Adjust the checkbox element on the form to TRUE (checkbox)
              $form["organism-$i"]['phenotype']['time']['time_phenotypes'][$time_phenotype]['#default_value'] = TRUE;
            }
          }
        }
      }
    }

    if (preg_match('/G/', $data_type)) {
      if ($i > 1) {
        $form["organism-$i"]['genotype-repeat-check'] = array(
          '#type' => 'checkbox',
          '#title' => "Genotype information for $name is the same as genotype information for {$form_state['saved_values'][TPPS_PAGE_1]['organism'][$i - 1]['name']}.",
          '#default_value' => isset($values["organism-$i"]['genotype-repeat-check']) ? $values["organism-$i"]['genotype-repeat-check'] : 1,
        );
      }

      $form["organism-$i"]['genotype'] = tpps_genotype($form, $form_state, $values, "organism-$i");

      if ($i > 1) {
        $form["organism-$i"]['genotype']['#states'] = array(
          'invisible' => array(
            ":input[name=\"organism-$i\[genotype-repeat-check]\"]" => array('checked' => TRUE),
          ),
        );
      }

    }

    if (preg_match('/E/', $data_type)) {
      if ($i > 1) {
        $form["organism-$i"]['environment-repeat-check'] = array(
          '#type' => 'checkbox',
          '#title' => "Environmental information for $name is the same as environmental information for {$form_state['saved_values'][TPPS_PAGE_1]['organism'][$i - 1]['name']}.",
          '#default_value' => isset($values["organism-$i"]['environment-repeat-check']) ? $values["organism-$i"]['environment-repeat-check'] : 1,
        );
      }

      $form["organism-$i"]['environment'] = tpps_environment($form, $form_state, "organism-$i");

      if ($i > 1) {
        $form["organism-$i"]['environment']['#states'] = array(
          'invisible' => array(
            ":input[name=\"organism-$i\[environment-repeat-check]\"]" => array('checked' => TRUE),
          ),
        );
      }
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission metadata.
  $meta = [
    'organism_number' => $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'],
    'data_type' => $form_state['saved_values'][TPPS_PAGE_2]['data_type'],
  ];
  tpps_add_buttons($form, 'page_4', $meta);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  // CHECK IF USER HAS CURATION ROLE
  global $user;

  // Check if the user has the 'curation' role.
  if (in_array('Curation', $user->roles)) {
    // [VS]
    // Temporary set weight of the Curation Diagnostics Tool to be below
    // navigation buttons. Will be removed when fix which moves JS and CSS
    // into separate files will be released.
    // Value must be greater then 100.
    $weight = 200;
    // [/VS].
    // DIAGNOSTIC UTILITIES FOR CURATION
    $form['diagnostics-curation'] = [
      '#type' => 'markup',
      '#markup' => '
        <h2 style="margin-top:10px;">🌟 Curation Diagnostics</h2>
        <div>These diagnostics <b>require you to save this page</b> with data
        before functions will work</div>
      ',
      '#weight' => $weight,
    ];

    $form['diagnostics-curation-style'] = [
      '#type' => 'markup',
      '#markup' => '
        <style>
          .cd-inline {
            display: inline-block;
          }
          .cd-inline-round-blue {
            display: inline-block;
            background-color: #00a5ff;
            color: white;
            padding: 3px;
            margin-top: 3px;
            margin-right: 3px;
            border-radius: 5px;
            width: 150px;
          }
          .cd-inline-round-red {
            display: inline-block;
            background-color: red;
            color: white;
            padding: 3px;
            margin-top: 3px;
            margin-right: 3px;
            border-radius: 5px;
            width: 150px;
          }
        </style>
      '
    ];
    $accession = $form_state['accession'];

    $js_onclick_code = "
      <script>
      function check_accession_file_tree_ids() {
        jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">⏰</h1>Checking Accession File Tree IDs...');
        jQuery.ajax({
          url: '/tpps/" . $accession . "/accession-file-tree-ids',
          error: function (err) {
            console.log(err);
            jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>It might be that this accession file is just too big to process it in time. Please contact Administration.');
          },        
          success: function (data) {
            console.log(data);
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '';
              html += '<div>';
              html += '🎄 Unique trees found: ' + data['unique_count'];
              html += ' | ';
              html += '🎄 Total trees found: ' + data['count'];
              html += '</div>';
              if (data['unique_count'] != data['count']) {
                html += '<div>⚡ There are duplicate tree IDs in this Accession file since unique count does not match count</div>';
                html += '<hr /><div>Duplicate Tree IDs (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['duplicate_values'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">' + data['duplicate_values'][i] + '</div>';
                }
              }
              else {
                html += '<div>🆗 No duplicate Tree IDs found in the Accession file</div>';
              }
              html += '<hr /><div>Unique Tree IDs (' + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
              }
              jQuery('#diagnostic-curation-results').html(html);
            }
            else {
              jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>No results returned, make sure you saved valid data on this page and retry. Double check Accession File existence as well.');
            }
          }
        });
      }
      </script>
    ";
    $form['button-check-accession-file-tree-ids'] = array(
      '#type' => 'button',
      '#prefix' => $js_onclick_code . '',
      '#value' => 'Check Accession File Tree IDs',
      '#attributes' => array(
        "onclick" => "javascript:check_accession_file_tree_ids(); return false;"
      ),
      '#weight' => $weight,
    );


    $js_onclick_code = "
      <script>
      function check_vcf_tree_ids() {
        jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">⏰</h1>Checking VCF Tree IDs...');
        jQuery.ajax({
          url: '/tpps/" . $accession . "/vcf-tree-ids',
          error: function (err) {
            console.log(err);
            jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>It might be that this VCF is just too big to process it in time. Please contact Administration.');
          },
          success: function (data) {
            console.log(data);
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '';
              html += '<div>';
              html += '🎄 Unique trees found: ' + data['unique_count'];
              html += ' | ';
              html += '🎄 Total trees found: ' + data['count'];
              html += '</div>';
              if (data['unique_count'] != data['count']) {
                html += '<div>⚡ There are duplicate tree IDs in this VCF since unique count does not match count</div>';
                html += '<hr /><div>Duplicate Tree IDs (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['duplicate_values'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">' + data['duplicate_values'][i] + '</div>';
                }
              }
              else {
                html += '<div>🆗 No duplicate Tree IDs found in the VCF file</div>';
              }
              html += '<hr /><div>Unique Tree IDs (' + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
              }
              jQuery('#diagnostic-curation-results').html(html);
            }
            else {
              jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>No results returned, make sure you saved valid data on this page and retry. Double check VCF existence as well.');
            }
          }
        });
      }
      </script>
    ";
    $form['button-check-vcf-tree-ids'] = array(
      '#type' => 'button',
      '#prefix' => $js_onclick_code . '',
      '#value' => 'Check VCF Tree IDs',
      '#attributes' => array(
        "onclick" => "javascript:check_vcf_tree_ids(); return false;"
      ),
      '#weight' => $weight,
    );


    $js_onclick_code = "
      <script>
      function compare_accession_tree_ids_vs_vcf_tree_ids() {
        jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">⏰</h1>Comparing Accession Tree IDs and VCF Tree IDs...');
        jQuery.ajax({
          url: '/tpps/" . $accession . "/compare-accession-file-vs-vcf-file-tree-ids',
          error: function (err) {
            console.log(err);
            jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>It might be that these files are just too big to process it in time. Please contact Administration.');
          },        
          success: function (data) {
            console.log(data);
            data = JSON.parse(data);
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '';
              html += '<div>';
              html += '🎄 None overlapping Accession trees found: ' + data['tree_ids_not_in_accession_count'];
              html += ' | ';
              html += '🎄 None overlapping VCF trees found: ' + data['tree_ids_not_in_vcf_count'];
              html += '</div>';
              if (data['tree_ids_not_in_accession'].length > 0) {
                html += '<div>⚡ There are VCF trees that do not overlap with the Accession file</div>';
                // html += '<hr /><div>Duplicate Tree IDs (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['tree_ids_not_in_accession'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">' + data['tree_ids_not_in_accession'][i] + '</div>';
                }
              }
              if (data['tree_ids_not_in_vcf'].length > 0) {
                html += '<div>⚡ There are Accession trees that do not overlap with the VCF file</div>';
                // html += '<hr /><div>Duplicate Tree IDs (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['tree_ids_not_in_vcf'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">' + data['tree_ids_not_in_vcf'][i] + '</div>';
                }
              }              
              // else {
              //   html += '<div>🆗 No duplicate Tree IDs found in the VCF file</div>';
              // }
              // html += '<hr /><div>Unique Tree IDs (' + data['values'].length + ')</div>';
              // for (var i=0; i<data['values'].length; i++) {
              //   html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
              // }
              jQuery('#diagnostic-curation-results').html(html);
            }
            else {
              jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>No results returned, make sure you saved valid data on this page and retry. Double check Accession and VCF existence as well.');
            }
          }
        });
      }
      </script>
    ";
    $form['button-compare-accession-tree-ids-vs-vcf-tree-ids'] = array(
      '#type' => 'button',
      '#prefix' => $js_onclick_code . '',
      '#value' => 'Compare Accession and VCF Tree IDs',
      '#attributes' => array(
        "onclick" => "javascript:compare_accession_tree_ids_vs_vcf_tree_ids(); return false;"
      ),
      '#weight' => $weight,
    ); 
   

    $form['curation-diagnostics-break-between-treeids-and-markers'] = array(
      '#type' => 'markup',
      '#markup' => '<br />'
    );    

    $js_onclick_code = "
      <script>
      function check_vcf_markers() {
        jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">⏰</h1>Checking VCF Markers...');
        jQuery.ajax({
          url: '/tpps/" . $accession . "/vcf-markers',
          error: function (err) {
            console.log(err);
            jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>It might be that this VCF is just too big to process it in time. Please contact Administration.');
          },
          success: function (data) {
            console.log(data);
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '';
              html += '<div>';
              html += '🧬 Unique markers found: ' + data['unique_count'];
              html += ' | ';
              html += '🧬 Total markers found: ' + data['count'];
              html += '</div>';
              if (data['unique_count'] != data['count']) {
                html += '<div>⚡ There are duplicate markers in this VCF since unique count does not match count</div>';
                html += '<hr /><div>Duplicate Markers (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['duplicate_values'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">' + data['duplicate_values'][i] + '</div>';
                }
              }
              else {
                html += '<div>🆗 No duplicate markers found in the VCF file</div>';
              }
              html += '<hr /><div>Unique Markers (' + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
              }
              jQuery('#diagnostic-curation-results').html(html);
            }
            else {
              jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>No results returned, make sure you saved valid data on this page and retry. Double check VCF existence as well.');
            }
          }
        });
      }
      </script>
    ";
    $form['button-check-vcf-markers'] = array(
      '#type' => 'button',
      '#prefix' => $js_onclick_code . '',
      '#value' => 'Check VCF Markers',
      '#attributes' => array(
        "onclick" => "javascript:check_vcf_markers(); return false;"
      ),
      '#weight' => $weight,
    );




    $js_onclick_code = "
      <script>
      function check_snps_assay_markers() {
        jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">⏰</h1>Checking SNPs Assay Markers...');
        jQuery.ajax({
          url: '/tpps/" . $accession . "/snps-assay-markers',
          error: function (err) {
            console.log(err);
            jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>It might be that this SNPs Assay is just too big to process it in time. Please contact Administration.');
          },
          success: function (data) {
            console.log(data);
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '';
              html += '<div>';
              html += '🧬 Unique markers found: ' + data['unique_count'];
              html += ' | ';
              html += '🧬 Total markers found: ' + data['count'];
              html += '</div>';
              if (data['unique_count'] != data['count']) {
                html += '<div>⚡ There are duplicate markers in this SNPs assay file since unique count does not match count</div>';
                html += '<hr /><div>Duplicate Markers (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['duplicate_values'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">' + data['duplicate_values'][i] + '</div>';
                }
              }
              else {
                html += '<div>🆗 No duplicate markers found in the SNPs Assay file</div>';
              }
              html += '<hr /><div>Unique Markers (' + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
              }
              jQuery('#diagnostic-curation-results').html(html);
            }
            else {
              jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1> No results returned, make sure you saved valid data on this page and retry. Double check SNPs Assay file existence as well.');
            }
          }
        });
      }
      </script>
    ";
    $form['button-check-snps-assay-markers'] = array(
      '#type' => 'button',
      '#prefix' => $js_onclick_code . '',
      '#value' => 'Check SNPs Assay Markers',
      '#attributes' => array(
        "onclick" => "javascript:check_snps_assay_markers(); return false;"
      ),
      '#weight' => $weight,
    );



    $js_onclick_code = "
      <script>
      function compare_vcf_markers_vs_snps_assay_markers() {
        jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">⏰</h1>Comparing VCF and SNPs Assay Markers...');
        jQuery.ajax({
          url: '/tpps/" . $accession . "/compare-vcf-markers-vs-snps-assay-markers',
          error: function (err) {
            console.log(err);
            jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>It might be that these files are just too big to process it in time. Please contact Administration.');
          },        
          success: function (data) {
            console.log(data);
            data = JSON.parse(data);
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '';
              html += '<div>';
              html += '🧬 None overlapping VCF markers found: ' + data['markers_not_in_snps_assay_count'];
              html += ' | ';
              html += '🧬 None overlapping SNPs Assay markers found: ' + data['markers_not_in_vcf_count'];
              html += '</div>';
              if (data['markers_not_in_snps_assay'].length > 0) {
                html += '<div>⚡ There are VCF markers that do not overlap with the SNPs Assay file</div>';
                // html += '<hr /><div>Duplicate Markers (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['markers_not_in_snps_assay'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">' + data['markers_not_in_snps_assay'][i] + '</div>';
                }
              }
              if (data['markers_not_in_vcf'].length > 0) {
                html += '<div>⚡ There are SNPs Assay markers that do not overlap with the VCF file</div>';
                // html += '<hr /><div>Duplicate Markers (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['markers_not_in_vcf'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">' + data['markers_not_in_vcf'][i] + '</div>';
                }
              }              
              // else {
              //   html += '<div>🆗 No duplicate Tree IDs found in the VCF file</div>';
              // }
              // html += '<hr /><div>Unique Tree IDs (' + data['values'].length + ')</div>';
              // for (var i=0; i<data['values'].length; i++) {
              //   html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
              // }
              jQuery('#diagnostic-curation-results').html(html);
            }
            else {
              jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>No results returned, make sure you saved valid data on this page and retry. Double check Accession and VCF existence as well.');
            }
          }
        });
      }
      </script>
    ";
    $form['button-compare-vcf-makers-vs-snps-assay-markers'] = array(
      '#type' => 'button',
      '#prefix' => $js_onclick_code . '',
      '#value' => 'Compare VCF and SNPs Assay markers',
      '#attributes' => array(
        "onclick" => "javascript:compare_vcf_markers_vs_snps_assay_markers(); return false;"
      ),
      '#weight' => $weight,
    ); 

    $form['diagnostic-curation-results'] = [
      '#type' => 'markup',
      '#markup' => '<div id="diagnostic-curation-results" style="max-height: 500px; overflow-y: auto;"></div>',
      '#weight' => $weight,
    ];
  }
}
