<?php

/**
 * @file
 * Define the helper functions for the GxPxE Data page.
 */

/**
 * Creates fields describing the phenotype data for the submission.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 * @param array $values
 *   The form_state values of the form to be populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 *
 * @return array
 *   The populated form.
 */
function phenotype(array &$form, array &$form_state, array $values, $id) {
  $phenotype_upload_location = 'public://' . variable_get('tpps_phenotype_files_dir', 'tpps_phenotype');

  $fields = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Phenotype Information:</div>'),
    '#tree' => TRUE,
    '#prefix' => "<div id=\"phenotypes-$id\">",
    '#suffix' => '</div>',
    '#description' => t('Upload a file and/or fill in form fields below to provide us with metadata about your phenotypes.'),
    '#collapsible' => TRUE,
  );

  $fields['iso-check'] = array(
    '#type' => 'checkbox',
    '#title' => t('My phenotypes are results from a mass spectrometry or isotope analysis'),
    '#ajax' => array(
      'callback' => 'update_phenotype',
      'wrapper' => "phenotypes-$id",
    ),
  );

  $iso_check = isset($form_state['values'][$id]['phenotype']['iso-check']) ? $form_state['values'][$id]['phenotype']['iso-check'] : NULL;
  if (!isset($iso_check)) {
    $iso_check = isset($form_state['saved_values'][TPPS_PAGE_4][$id]['phenotype']['iso-check']) ? $form_state['saved_values'][TPPS_PAGE_4][$id]['phenotype']['iso-check'] : NULL;
  }

  if (!empty($iso_check)) {
    $fields['iso'] = array(
      '#type' => 'managed_file',
      '#title' => t('Phenotype Isotope/Mass Spectrometry file: *'),
      '#upload_location' => $phenotype_upload_location,
      '#upload_validators' => array(
        'file_validate_extensions' => array('csv tsv xlsx'),
      ),
      '#description' => 'Please upload a file containing all of your isotope/mass spectrometry data. The format of this file is very important! The first column of your file should contain tree identifiers which match the tree identifiers you provided in your tree accession file, and all of the remaining columns should contain isotope or mass spectrometry data.'
    );

    return $fields;
  }

  if (isset($form_state['values'][$id]['phenotype']['number']) and $form_state['triggering_element']['#name'] == "Add Phenotype-$id") {
    $form_state['values'][$id]['phenotype']['number']++;
  }
  elseif (isset($form_state['values'][$id]['phenotype']['number']) and $form_state['triggering_element']['#name'] == "Remove Phenotype-$id" and $form_state['values'][$id]['phenotype']['number'] > 0) {
    $form_state['values'][$id]['phenotype']['number']--;
  }
  $phenotype_number = isset($form_state['values'][$id]['phenotype']['number']) ? $form_state['values'][$id]['phenotype']['number'] : NULL;

  if (!isset($phenotype_number) and isset($form_state['saved_values'][TPPS_PAGE_4][$id]['phenotype']['number'])) {
    $phenotype_number = $form_state['saved_values'][TPPS_PAGE_4][$id]['phenotype']['number'];
  }
  if (!isset($phenotype_number)) {
    $phenotype_number = 0;
  }

  $fields['add'] = array(
    '#type' => 'button',
    '#name' => t("Add Phenotype-@i", array('@i' => $id)),
    '#button_type' => 'button',
    '#value' => t('Add Phenotype'),
    '#ajax' => array(
      'callback' => 'update_phenotype',
      'wrapper' => "phenotypes-$id",
    ),
  );

  $fields['remove'] = array(
    '#type' => 'button',
    '#name' => t("Remove Phenotype-@i", array('@i' => $id)),
    '#button_type' => 'button',
    '#value' => t('Remove Phenotype'),
    '#ajax' => array(
      'callback' => 'update_phenotype',
      'wrapper' => "phenotypes-$id",
    ),
  );

  $fields['number'] = array(
    '#type' => 'hidden',
    '#value' => "$phenotype_number",
  );

  $fields['phenotypes-meta'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );

  for ($i = 1; $i <= $phenotype_number; $i++) {

    $fields['phenotypes-meta']["$i"] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );

    $fields['phenotypes-meta']["$i"]['name'] = array(
      '#type' => 'textfield',
      '#title' => t("Phenotype @i Name: *", array('@i' => $i)),
      '#autocomplete_path' => 'phenotype/autocomplete',
      '#prefix' => "<label><b>Phenotype $i:</b></label>",
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('right'),
        'title' => array('If your phenotype name is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.'),
      ),
      '#description' => t('Phenotype "name" is the human-readable name of the phenotype, where "attribute" is the thing that the phenotype is describing. Phenotype "name" should match the data in the "Phenotype Name/Identifier" column that you select in your <a href="@url">Phenotype file</a> below.', array('@url' => url('/tpps', array('fragment' => "edit-$id-phenotype-file-ajax-wrapper")))),
    );

    $fields['phenotypes-meta']["$i"]['attribute'] = array(
      '#type' => 'textfield',
      '#title' => t("Phenotype @i Attribute: *", array('@i' => $i)),
      '#autocomplete_path' => 'attribute/autocomplete',
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('right'),
        'title' => array('If your attribute is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.'),
      ),
      '#description' => t('Some examples of attributes include: "amount", "width", "mass density", "area", "height", "age", "broken", "time", "color", "composition", etc.'),
    );

    $fields['phenotypes-meta']["$i"]['description'] = array(
      '#type' => 'textfield',
      '#title' => t("Phenotype @i Description: *", array('@i' => $i)),
      '#description' => t("Please provide a short description of Phenotype @i", array('@i' => $i)),
    );

    $fields['phenotypes-meta']["$i"]['units'] = array(
      '#type' => 'textfield',
      '#title' => t("Phenotype @i Units: *", array('@i' => $i)),
      '#autocomplete_path' => 'units/autocomplete',
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('right'),
        'title' => array('If your unit is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.'),
      ),
      '#description' => t('Some examples of units include: "m", "meters", "in", "inches", "Degrees Celsius", "°C", etc.'),
    );

    $fields['phenotypes-meta']["$i"]['struct-check'] = array(
      '#type' => 'checkbox',
      '#title' => t("Phenotype @i has a structure descriptor", array('@i' => $i)),
    );

    $fields['phenotypes-meta']["$i"]['structure'] = array(
      '#type' => 'textfield',
      '#title' => t("Phenotype @i Structure: *", array('@i' => $i)),
      '#autocomplete_path' => 'structure/autocomplete',
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('right'),
        'title' => array('If your structure is not in the autocomplete list, don\'t worry about it! We will create new phenotype metadata in the database for you.'),
      ),
      '#description' => t('Some examples of structure descriptors include: "stem", "bud", "leaf", "xylem", "whole plant", "meristematic apical cell", etc.'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[phenotype][phenotypes-meta][' . $i . '][struct-check]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $fields['phenotypes-meta']["$i"]['val-check'] = array(
      '#type' => 'checkbox',
      '#title' => t("Phenotype @i has a value range", array('@i' => $i)),
    );

    $fields['phenotypes-meta']["$i"]['min'] = array(
      '#type' => 'textfield',
      '#title' => t("Phenotype @i Minimum Value (type 1 for binary): *", array('@i' => $i)),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[phenotype][phenotypes-meta][' . $i . '][val-check]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $fields['phenotypes-meta']["$i"]['max'] = array(
      '#type' => 'textfield',
      '#title' => t("Phenotype @i Maximum Value (type 2 for binary): *", array('@i' => $i)),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[phenotype][phenotypes-meta][' . $i . '][val-check]"]' => array('checked' => TRUE),
        ),
      ),
    );
  }

  $fields['check'] = array(
    '#type' => 'checkbox',
    '#title' => t('I would like to upload a phenotype metadata file'),
    '#attributes' => array(
      'data-toggle' => array('tooltip'),
      'data-placement' => array('right'),
      'title' => array('Upload a file'),
    ),
    '#description' => t('We encourage that you only upload a phenotype metadata file if you have > 20 phenotypes. Using the fields above instead of uploading a metadata file allows you to select from standardized controlled vocabulary terms, which makes your data more findable, interoperable, and reusable.'),
  );

  $fields['metadata'] = array(
    '#type' => 'managed_file',
    '#title' => t('Phenotype Metadata File: Please upload a file containing columns with the name, attribute, description, and units of each of your phenotypes: *'),
    '#upload_location' => "$phenotype_upload_location",
    '#upload_validators' => array(
      'file_validate_extensions' => array('csv tsv xlsx'),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE),
      ),
    ),
    '#tree' => TRUE,
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
    'Maximum Value',
  );

  $fields['metadata']['columns-options'] = array(
    '#type' => 'hidden',
    '#value' => $column_options,
  );

  $fields['metadata']['no-header'] = array();

  return $fields;
}

/**
 * Creates fields describing the genotype data for the submission.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 * @param array $values
 *   The form_state values of the form to be populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 *
 * @return array
 *   The populated form.
 */
function genotype(array &$form, array &$form_state, array $values, $id) {

  $genotype_upload_location = 'public://' . variable_get('tpps_genotype_files_dir', 'tpps_genotype');

  $fields = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Genotype Information:</div>'),
    '#collapsible' => TRUE,
  );

  page_4_marker_info($fields, $id);

  page_4_ref($fields, $form_state, $id);

  if (isset($form_state['complete form'][$id]['genotype']['marker-type']['SNPs']['#value'])) {
    $snps_check = $form_state['complete form'][$id]['genotype']['marker-type']['SNPs']['#value'];
  }
  elseif (isset($form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['marker-type']['SNPs'])) {
    $snps_check = $form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['marker-type']['SNPs'];
  }

  if (isset($form_state['complete form'][$id]['genotype']['marker-type']['SSRs/cpSSRs']['#value'])) {
    $ssrs_check = $form_state['complete form'][$id]['genotype']['marker-type']['SSRs/cpSSRs']['#value'];
  }
  elseif (isset($form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['marker-type']['SSRs/cpSSRs'])) {
    $ssrs_check = $form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['marker-type']['SSRs/cpSSRs'];
  }

  if (isset($form_state['complete form'][$id]['genotype']['marker-type']['Other']['#value'])) {
    $other_marker_check = $form_state['complete form'][$id]['genotype']['marker-type']['Other']['#value'];
  }
  elseif (isset($form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['marker-type']['Other'])) {
    $other_marker_check = $form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['marker-type']['Other'];
  }

  $fields['files'] = array(
    '#type' => 'fieldset',
    '#prefix' => "<div id='$id-genotype-files'>",
    '#suffix' => '</div>',
  );

  if (!empty($ssrs_check)) {
    $fields['files']['ploidy'] = array(
      '#type' => 'select',
      '#title' => t('Ploidy'),
      '#options' => array(
        0 => '- Select -',
        'Haploid' => 'Haploid',
        'Diploid' => 'Diploid',
        'Polyploid' => 'Polyploid',
      ),
      '#ajax' => array(
        'callback' => 'genotype_files_callback',
        'wrapper' => "$id-genotype-files",
      ),
    );
  }

  $options = array();
  if (!empty($snps_check)) {
    $options['SNPs Genotype Assay'] = 'SNPs Genotype Assay';
    if (!empty($form_state['values'][$id]['genotype']['files']['file-type']['SNPs Genotype Assay']) or !empty($form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['file-type']['SNPs Genotype Assay'])) {
      $options['Assay Design'] = 'Assay Design';
    }

    if (isset($form_state['complete form'][$id]['genotype']['files']['file-type']['SNPs Genotype Assay']['#value'])) {
      $snps_assay_check = $form_state['complete form'][$id]['genotype']['files']['file-type']['SNPs Genotype Assay']['#value'];
    }
    elseif (isset($form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['file-type']['SNPs Genotype Assay'])) {
      $snps_assay_check = $form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['file-type']['SNPs Genotype Assay'];
    }

    if (!empty($snps_assay_check) and isset($form_state['complete form'][$id]['genotype']['files']['file-type']['Assay Design']['#value'])) {
      $assay_design_check = $form_state['complete form'][$id]['genotype']['files']['file-type']['Assay Design']['#value'];
    }
    elseif (!empty($snps_assay_check) and isset($form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['file-type']['Assay Design'])) {
      $assay_design_check = $form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['file-type']['Assay Design'];
    }
  }
  if (!empty($ssrs_check)) {
    $options['SSRs/cpSSRs Genotype Spreadsheet'] = 'SSRs/cpSSRs Genotype Spreadsheet';

    if (isset($form_state['complete form'][$id]['genotype']['files']['file-type']['SSRs/cpSSRs Genotype Spreadsheet']['#value'])) {
      $ssrs_file_check = $form_state['complete form'][$id]['genotype']['files']['file-type']['SSRs/cpSSRs Genotype Spreadsheet']['#value'];
    }
    elseif (isset($form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['file-type']['SSRs/cpSSRs Genotype Spreadsheet'])) {
      $ssrs_file_check = $form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['file-type']['SSRs/cpSSRs Genotype Spreadsheet'];
    }
  }
  if (!empty($other_marker_check)) {
    $options['Other Marker Genotype Spreadsheet'] = 'Other Marker Genotype Spreadsheet';

    if (isset($form_state['complete form'][$id]['genotype']['files']['file-type']['Other Marker Genotype Spreadsheet']['#value'])) {
      $other_file_check = $form_state['complete form'][$id]['genotype']['files']['file-type']['Other Marker Genotype Spreadsheet']['#value'];
    }
    elseif (isset($form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['file-type']['Other Marker Genotype Spreadsheet'])) {
      $other_file_check = $form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['file-type']['Other Marker Genotype Spreadsheet'];
    }
  }
  $options['VCF'] = 'VCF';

  $fields['files']['file-type'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Genotype File Types (select all that apply): *'),
    '#options' => $options,
    '#ajax' => array(
      'callback' => 'genotype_files_callback',
      'wrapper' => "$id-genotype-files",
    ),
  );

  if (isset($form_state['complete form'][$id]['genotype']['files']['file-type']['VCF']['#value'])) {
    $vcf_file_check = $form_state['complete form'][$id]['genotype']['files']['file-type']['VCF']['#value'];
  }
  elseif (isset($form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['file-type']['VCF'])) {
    $vcf_file_check = $form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['files']['file-type']['VCF'];
  }

  if (!empty($snps_assay_check)) {
    $fields['files']['snps-assay'] = array(
      '#type' => 'managed_file',
      '#title' => t('SNPs Genotype Assay File: please provide a spreadsheet with columns for the Tree ID of genotypes used in this study: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('xlsx'),
      ),
      '#description' => "Please upload a spreadsheet file containing SNP Genotype Assay data. The format of this file is very important! The first column of your file should contain tree identifiers which match the tree identifiers you provided in your tree accession file, and all of the remaining columns should contain SNP data.",
      '#tree' => TRUE,
    );

    if (isset($fields['files']['snps-assay']['#value']['fid'])) {
      $fields['files']['snps-assay']['#default_value'] = $fields['files']['snps-assay']['#value']['fid'];
    }
    if (!empty($fields['files']['snps-assay']['#default_value']) and ($file = file_load($fields['files']['snps-assay']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }
  }
  else {
    $fields['files']['snps-assay'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  if (!empty($assay_design_check)) {
    $fields['files']['assay-design'] = array(
      '#type' => 'managed_file',
      '#title' => 'Genotype Assay Design File: *',
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('xlsx'),
      ),
      '#tree' => TRUE,
    );

    if (isset($fields['files']['assay-design']['#value'])) {
      $fields['files']['assay-design']['#default_value'] = $fields['files']['assay-design']['#value'];
    }
    if (!empty($fields['files']['assay-design']['#default_value']) and ($file = file_load($fields['files']['assay-design']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }
  }
  else {
    $fields['files']['assay-design'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  if (!empty($ssrs_file_check)) {
    $fields['files']['ssrs'] = array(
      '#type' => 'managed_file',
      '#title' => t('SSRs/cpSSRs Spreadsheet: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('xlsx'),
      ),
      '#description' => t('Please upload a spreadsheet containing your SSRs/cpSSRs data. The format of this file is very important! TPPS will parse your file based on the ploidy you have selected above. For any ploidy, TPPS will assume that the first column of your file is the column that holds the Tree Identifier that matches your accession file.'),
      '#tree' => TRUE,
    );

    if (isset($form_state['values'][$id]['genotype']['files']['ploidy'])) {
      switch ($form_state['values'][$id]['genotype']['files']['ploidy']) {
        case 'Haploid':
          $fields['files']['ssrs']['#description'] .= ' For haploid, TPPS assumes that each remaining column in the spreadsheet is a marker.';
          break;

        case 'Diploid':
          $fields['files']['ssrs']['#description'] .= ' For diploid, TPPS will assume that pairs of columns together are describing an individual marker, so the second and third columns would be the first marker, the fourth and fifth columns would be the second marker, etc.';
          break;

        case 'Polyploid':
          $fields['files']['ssrs']['#description'] .= ' For polyploid, TPPS will read columns until it arrives at a non-empty column with a different name from the last.';
          break;

        default:
          break;
      }
    }

    if (isset($fields['files']['ssrs']['#value']['fid'])) {
      $fields['files']['ssrs']['#default_value'] = $fields['files']['ssrs']['#value']['fid'];
    }
    if (!empty($fields['files']['ssrs']['#default_value']) and ($file = file_load($fields['files']['ssrs']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }
  }
  else {
    $fields['files']['ssrs'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  if (!empty($other_file_check)) {
    $fields['files']['other'] = array(
      '#type' => 'managed_file',
      '#title' => t('Other Marker Genotype Spreadsheet: please provide a spreadsheet with columns for the Tree ID of genotypes used in this study: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('xlsx'),
      ),
      '#description' => "Please upload a spreadsheet file containing Genotype data. When your file is uploaded, you will be shown a table with your column header names, several drop-downs, and the first few rows of your file. You will be asked to define the data type for each column, using the drop-downs provided to you. If a column data type does not fit any of the options in the drop-down menu, you may set that drop-down menu to \"N/A\". Your file must contain one column with the Tree Identifier.",
      '#tree' => TRUE,
    );

    $fields['files']['other']['empty'] = array(
      '#default_value' => isset($values[$id]['genotype']['files']['other']['empty']) ? $values[$id]['genotype']['files']['other']['empty'] : 'NA',
    );

    $fields['files']['other']['columns'] = array(
      '#description' => 'Please define which columns hold the required data: Tree Identifier, Genotype Data',
    );

    $fields['files']['other']['columns-options'] = array(
      '#type' => 'hidden',
      '#value' => array(
        'Genotype Data',
        'Tree Identifier',
        'N/A',
      ),
    );

    $fields['files']['other']['no-header'] = array();
  }
  else {
    $fields['files']['other'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  if (!empty($vcf_file_check)) {
    $fields['files']['vcf'] = array(
      '#type' => 'managed_file',
      '#title' => t('Genotype VCF File: *'),
      '#upload_location' => "$genotype_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('vcf'),
      ),
      '#tree' => TRUE,
    );

    if (isset($fields['files']['vcf']['#value'])) {
      $fields['files']['vcf']['#default_value'] = $fields['files']['vcf']['#value'];
    }
    if (!empty($fields['files']['vcf']['#default_value']) and ($file = file_load($fields['files']['vcf']['#default_value']))) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }
  }
  else {
    $fields['files']['vcf'] = array(
      '#type' => 'managed_file',
      '#tree' => TRUE,
      '#access' => FALSE,
    );
  }

  return $fields;
}

/**
 * Creates fields describing the environmental data for the submission.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 *
 * @return array
 *   The populated form.
 */
function environment(array &$form, array &$form_state, $id) {
  $cartogratree_env = variable_get('tpps_cartogratree_env', FALSE);

  $fields = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Environmental Information:</div>'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
    '#prefix' => "<div id=\"environment-$id\">",
    '#suffix' => '</div>',
  );

  if ($cartogratree_env and db_table_exists('cartogratree_groups') and db_table_exists('cartogratree_layers') and db_table_exists('cartogratree_fields')) {

    $query = db_select('variable', 'v')
      ->fields('v')
      ->condition('name', db_like('tpps_layer_group_') . '%', 'LIKE');

    $results = $query->execute();
    $options = array();

    while (($result = $results->fetchObject())) {
      $group_id = substr($result->name, 17);
      $group = db_select('cartogratree_groups', 'g')
        ->fields('g', array('group_id', 'group_name'))
        ->condition('group_id', $group_id)
        ->execute()
        ->fetchObject();
      $group_is_enabled = variable_get("tpps_layer_group_$group_id", FALSE);

      if ($group_is_enabled) {
        if ($group->group_name == 'WorldClim v.2 (WorldClim)') {
          $subgroups_query = db_select('cartogratree_layers', 'c')
            ->distinct()
            ->fields('c', array('subgroup_id'))
            ->condition('c.group_id', $group_id)
            ->execute();
          while (($subgroup = $subgroups_query->fetchObject())) {
            $subgroup_title = db_select('cartogratree_subgroups', 's')
              ->fields('s', array('subgroup_name'))
              ->condition('subgroup_id', $subgroup->subgroup_id)
              ->execute()
              ->fetchObject()->subgroup_name;
            $options["worldclim_subgroup_{$subgroup->subgroup_id}"] = array(
              'group_id' => $group_id,
              'group' => $group->group_name,
              'title' => $subgroup_title,
              'params' => NULL,
            );
          }
        }
        else {
          $layers_query = db_select('cartogratree_layers', 'c')
            ->fields('c', array('title', 'group_id', 'layer_id'))
            ->condition('c.group_id', $group_id);
          $layers_results = $layers_query->execute();
          while (($layer = $layers_results->fetchObject())) {
            $params_query = db_select('cartogratree_fields', 'f')
              ->fields('f', array('display_name', 'field_id'))
              ->condition('f.layer_id', $layer->layer_id);
            $params_results = $params_query->execute();
            $params = array();
            while (($param = $params_results->fetchObject())) {
              $params[$param->field_id] = $param->display_name;
            }
            $options[$layer->layer_id] = array(
              'group_id' => $layer->group_id,
              'group' => $group->group_name,
              'title' => $layer->title,
              'params' => $params,
            );
          }
        }
      }
    }

    $fields['use_layers'] = array(
      '#type' => 'checkbox',
      '#title' => 'I used environmental layers in my study that are indexed by CartograTree.',
      '#description' => 'If the layer you used is not in the list below, then the administrator for this site might not have enabled the layer group you used. Please contact them for more information.',
    );

    $fields['env_layers_groups'] = array(
      '#type' => 'fieldset',
      '#title' => 'Cartogratree Environmental Layers: *',
      '#collapsible' => TRUE,
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[environment][use_layers]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $fields['env_layers'] = array(
      '#type' => 'fieldset',
      '#title' => 'Cartogratree Environmental Layers: *',
      '#collapsible' => TRUE,
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[environment][use_layers]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $fields['env_params'] = array(
      '#type' => 'fieldset',
      '#title' => 'CartograTree Environmental Layer Parameters: *',
      '#collapsible' => TRUE,
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[environment][use_layers]"]' => array('checked' => TRUE),
        ),
      ),
    );

    foreach ($options as $layer_id => $layer_info) {
      $layer_title = $layer_info['title'];
      $layer_group = $layer_info['group'];
      $layer_params = $layer_info['params'];

      $fields['env_layers_groups'][$layer_group] = array(
        '#type' => 'checkbox',
        '#title' => $layer_group,
        '#return_value' => $layer_info['group_id'],
      );

      $fields['env_layers'][$layer_title] = array(
        '#type' => 'checkbox',
        '#title' => "<strong>$layer_title</strong> - $layer_group",
        '#states' => array(
          'visible' => array(
            ':input[name="' . $id . '[environment][env_layers_groups][' . $layer_group . ']"]' => array('checked' => TRUE),
          ),
        ),
        '#return_value' => $layer_id,
      );

      if (!empty($layer_params)) {
        $fields['env_params']["$layer_title"] = array(
          '#type' => 'fieldset',
          '#title' => "$layer_title Parameters",
          '#description' => "Please select the parameters you used from the $layer_title layer.",
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[environment][env_layers_groups][' . $layer_group . ']"]' => array('checked' => TRUE),
              ':input[name="' . $id . '[environment][env_layers][' . $layer_title . ']"]' => array('checked' => TRUE),
            ),
          ),
        );

        foreach ($layer_params as $param_id => $param) {
          $fields['env_params']["$layer_title"][$param] = array(
            '#type' => 'checkbox',
            '#title' => $param,
            '#return_value' => $param_id,
          );
        }
      }
    }
  }

  $fields['env_manual_check'] = array(
    '#type' => 'checkbox',
    '#title' => 'I have environmental data that I collected myself.',
  );

  if (isset($form_state['values'][$id]['environment']['number']) and $form_state['triggering_element']['#name'] == "Add Environment Data-$id") {
    $form_state['values'][$id]['environment']['number']++;
  }
  elseif (isset($form_state['values'][$id]['environment']['number']) and $form_state['triggering_element']['#name'] == "Remove Environment Data-$id" and $form_state['values'][$id]['environment']['number'] > 0) {
    $form_state['values'][$id]['environment']['number']--;
  }
  $environment_number = isset($form_state['values'][$id]['environment']['number']) ? $form_state['values'][$id]['environment']['number'] : NULL;

  if (!isset($environment_number) and isset($form_state['saved_values'][TPPS_PAGE_4][$id]['environment']['number'])) {
    $environment_number = $form_state['saved_values'][TPPS_PAGE_4][$id]['environment']['number'];
  }
  if (!isset($environment_number)) {
    $environment_number = 1;
  }

  $fields['number'] = array(
    '#type' => 'hidden',
    '#value' => "$environment_number",
  );

  $fields['env_manual'] = array(
    '#type' => 'fieldset',
    '#title' => 'Custom Environmental Data:',
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[environment][env_manual_check]"]' => array('checked' => TRUE),
      ),
    ),
    '#collapsible' => TRUE,
  );

  $fields['env_manual']['add'] = array(
    '#type' => 'button',
    '#name' => t("Add Environment Data-@i", array('@i' => $id)),
    '#button_type' => 'button',
    '#value' => t('Add Environment Data'),
    '#ajax' => array(
      'callback' => 'update_environment',
      'wrapper' => "environment-$id",
    ),
  );

  $fields['env_manual']['remove'] = array(
    '#type' => 'button',
    '#name' => t("Remove Environment Data-@i", array('@i' => $id)),
    '#button_type' => 'button',
    '#value' => t('Remove Environment Data'),
    '#ajax' => array(
      'callback' => 'update_environment',
      'wrapper' => "environment-$id",
    ),
  );

  for ($i = 1; $i <= $environment_number; $i++) {

    $fields['env_manual']["$i"] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );

    $fields['env_manual']["$i"]['name'] = array(
      '#type' => 'textfield',
      '#title' => t("Environmental Data @i Name: *", array('@i' => $i)),
      '#prefix' => "<label><b>Environment Data $i:</b></label>",
      '#description' => t('Please provide the name of Environmental Data @i. Some example environmental data names might include "soil chemistry", "rainfall", "average temperature", etc.', array('@i' => $i)),
    );

    $fields['env_manual']["$i"]['description'] = array(
      '#type' => 'textfield',
      '#title' => t("Environmental Data @i Description: *", array('@i' => $i)),
      '#description' => t("Please provide a short description of Environmental Data @i.", array('@i' => $i)),
    );

    $fields['env_manual']["$i"]['units'] = array(
      '#type' => 'textfield',
      '#title' => t("Environmental Data @i Units: *", array('@i' => $i)),
      '#description' => t("Please provide the units of Environmental Data @i.", array('@i' => $i)),
    );

    $fields['env_manual']["$i"]['value'] = array(
      '#type' => 'textfield',
      '#title' => t("Environmental Data @i Value: *", array('@i' => $i)),
      '#description' => t("Please provide the value of Environmental Data @i.", array('@i' => $i)),
    );
  }

  return $fields;
}

/**
 * Creates fields for the user to specify a reference genome.
 *
 * @param array $fields
 *   The form element being populated.
 * @param array $form_state
 *   The state of the form to be populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 *
 * @global stdClass $user
 *   The user accessing the form.
 */
function page_4_ref(array &$fields, array &$form_state, $id) {
  global $user;
  $uid = $user->uid;

  $options = array(
    'key' => 'filename',
    'recurse' => FALSE,
  );

  $genome_dir = variable_get('tpps_local_genome_dir', NULL);
  $ref_genome_arr = array();
  $ref_genome_arr[0] = '- Select -';

  if ($genome_dir) {
    $results = file_scan_directory($genome_dir, '/^([A-Z][a-z]{3})$/', $options);
    foreach ($results as $key => $value) {
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
      foreach ($versions as $item) {
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
        ':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'bio'),
      ),
    ),
  );

  $fields['assembly-auto'] = array(
    '#type' => 'fieldset',
    '#title' => t('Waiting for BioProject accession number...'),
    '#tree' => TRUE,
    '#prefix' => "<div id='$id-assembly-auto'>",
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'bio'),
      ),
    ),
  );

  if (isset($form_state['values'][$id]['genotype']['BioProject-id']) and $form_state['values'][$id]['genotype']['BioProject-id'] != '') {
    $bio_id = $form_state['values']["$id"]['genotype']['BioProject-id'];
    $form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['BioProject-id'] = $form_state['values'][$id]['genotype']['BioProject-id'];
  }
  elseif (isset($form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['BioProject-id']) and $form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['BioProject-id'] != '') {
    $bio_id = $form_state['saved_values'][TPPS_PAGE_4][$id]['genotype']['BioProject-id'];
  }
  elseif (isset($form_state['complete form']['organism-1']['genotype']['BioProject-id']['#value']) and $form_state['complete form']['organism-1']['genotype']['BioProject-id']['#value'] != '') {
    $bio_id = $form_state['complete form']['organism-1']['genotype']['BioProject-id']['#value'];
  }

  if (isset($bio_id) and $bio_id != '') {

    if (strlen($bio_id) > 5) {
      $bio_id = substr($bio_id, 5);
    }

    $options = array();
    $url = "https://eutils.ncbi.nlm.nih.gov/entrez/eutils/elink.fcgi?dbfrom=bioproject&db=nuccore&id=" . $bio_id;
    $response_xml_data = file_get_contents($url);
    $link_types = simplexml_load_string($response_xml_data)->children()->children()->LinkSetDb;

    if (preg_match('/<LinkSetDb>/', $response_xml_data)) {

      foreach ($link_types as $type_xml) {
        $type = $type_xml->LinkName->__tostring();

        switch ($type) {
          case 'bioproject_nuccore_tsamaster':
            $suffix = 'TSA';
            break;

          case 'bioproject_nuccore_wgsmaster':
            $suffix = 'WGS';
            break;

          default:
            continue 2;
        }

        foreach ($type_xml->Link as $link) {
          $options[$link->Id->__tostring()] = $suffix;
        }
      }

      $fields['assembly-auto']['#title'] = '<div class="fieldset-title">Select all that apply: *</div>';
      $fields['assembly-auto']['#collapsible'] = TRUE;

      foreach ($options as $item => $suffix) {
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
  $fasta['#type'] = 'fieldset';
  $fasta['#title'] = 'Tripal FASTA Loader';
  $fasta['#states'] = array(
    'visible' => array(
    array(
      array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'url')),
      'or',
      array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual')),
      'or',
      array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual2')),
    ),
    ),
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
      'file_validate_extensions' => array(implode(' ', $class::$file_types)),
    ),
    '#upload_location' => $tripal_upload_location,
  );

  $fasta['file']['file_upload'] = $upload;
  $fasta['analysis_id']['#required'] = $fasta['seqtype']['#required'] = FALSE;
  $fasta['file']['file_upload']['#states'] = $fasta['file']['file_upload_existing']['#states'] = array(
    'visible' => array(
    array(
      array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual')),
      'or',
      array(':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual2')),
    ),
    ),
  );
  $fasta['file']['file_remote']['#states'] = array(
    'visible' => array(
      ':input[name="]' . $id . '[genotype][ref-genome]"]' => array('value' => 'url'),
    ),
  );

  $fields['tripal_fasta'] = $fasta;
}

/**
 * Creates fields describing the genotype markers used in the submission.
 *
 * @param array $fields
 *   The form element being populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 */
function page_4_marker_info(array &$fields, $id) {

  $fields['marker-type'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Marker Type (select all that apply): *'),
    '#options' => drupal_map_assoc(array(
      t('SNPs'),
      t('SSRs/cpSSRs'),
      t('Other'),
    )),
  );

  $fields['marker-type']['#ajax'] = array(
    'callback' => 'genotype_files_callback',
    'wrapper' => "$id-genotype-files",
  );

  $fields['SNPs'] = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">SNPs Information:</div>'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][marker-type][SNPs]"]' => array('checked' => TRUE),
      ),
    ),
    '#collapsible' => TRUE,
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
      5 => 'Genotyping Array',
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
      5 => 'Other',
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '1'),
      ),
    ),
  );

  $fields['SNPs']['GBS-other'] = array(
    '#type' => 'textfield',
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][SNPs][GBS]"]' => array('value' => '5'),
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '1'),
      ),
    ),
  );

  $fields['SNPs']['targeted-capture'] = array(
    '#type' => 'select',
    '#title' => t('Targeted Capture Type: *'),
    '#options' => array(
      0 => '- Select -',
      1 => 'Exome Capture',
      2 => 'Other',
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '2'),
      ),
    ),
  );

  $fields['SNPs']['targeted-capture-other'] = array(
    '#type' => 'textfield',
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][SNPs][targeted-capture]"]' => array('value' => '2'),
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '2'),
      ),
    ),
  );

  $fields['SSRs/cpSSRs'] = array(
    '#type' => 'textfield',
    '#title' => t('Define SSRs/cpSSRs Type: *'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][marker-type][SSRs/cpSSRs]"]' => array('checked' => TRUE),
      ),
    ),
  );

  $fields['other-marker'] = array(
    '#type' => 'textfield',
    '#title' => t('Define Other Marker Type: *'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][marker-type][Other]"]' => array('checked' => TRUE),
      ),
    ),
  );
}
