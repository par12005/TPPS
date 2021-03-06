<?php

/**
 * @file
 * Defines functions to create and populate Zenodo DOIs.
 */

/**
 * Creates a Zenodo DOI and loads metadata from a TPPS Submission accession.
 *
 * @param string $accession
 *   The accession number of the TPPS Submission.
 *
 * @global string $base_url
 *   The base URL of the site.
 *
 * @return object
 *   The Zenodo DOI Deposition object.
 */
function tpps_generate_doi($accession) {
  global $base_url;
  $state = tpps_load_submission($accession);

  $id = tpps_doi_create();
  $creators = array(
    (object) array(
      'name' => tpps_doi_format_name($state['saved_values'][TPPS_PAGE_1]['primaryAuthor']),
      'affiliation' => $state['saved_values'][TPPS_PAGE_1]['organization'],
    ),
  );
  for ($i = 1; $i <= $state['saved_values'][TPPS_PAGE_1]['publication']['secondaryAuthors']['number']; $i++) {
    $creators[] = (object) array(
      'name' => tpps_doi_format_name($state['saved_values'][TPPS_PAGE_1]['publication']['secondaryAuthors'][$i]),
    );
  }

  $keywords = array();
  for ($i = 1; $i <= $state['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
    $keywords[] = $state['saved_values'][TPPS_PAGE_1]['organism'][$i]['name'];
  }

  $study_type_options = array(
    1 => 'Natural Population (Landscape)',
    2 => 'Growth Chamber',
    3 => 'Greenhouse',
    4 => 'Experimental/Common Garden',
    5 => 'Plantation',
  );

  $keywords[] = $study_type_options[$state['saved_values'][TPPS_PAGE_2]['study_type']];

  $metadata = (object) array(
    'title' => $state['saved_values'][TPPS_PAGE_1]['publication']['title'],
    'upload_type' => 'dataset',
    'description' => $state['saved_values'][TPPS_PAGE_1]['publication']['abstract'],
    'creators' => $creators,
    // 'access_right' => 'embargoed', //TODO: collect embargo date from user.
    'keywords' => $keywords,
    'notes' => $state['saved_values']['summarypage']['comments'],
    'related_identifiers' => array(
      (object) array(
        'relation' => 'isAlternateIdentifier',
        'identifier' => "$base_url/tpps/details/$accession",
      ),
    ),
  );

  tpps_doi_upload_files($id, $state);

  $upload = tpps_doi_metadata($id, $metadata);

  return $upload;
}

/**
 * Creates a blank Zenodo DOI Deposition. Returns the Zenodo id number.
 *
 * @return int
 *   The Zenodo Deposition id number.
 */
function tpps_doi_create() {
  $token = variable_get('tpps_zenodo_api_key', NULL);
  $prefix = variable_get('tpps_zenodo_prefix', '');
  $ch = curl_init("https://{$prefix}zenodo.org/api/deposit/depositions?" . http_build_query(array('access_token' => $token)));
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json'));
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_setopt($ch, CURLOPT_POSTFIELDS, '{}');

  $out = json_decode(curl_exec($ch));
  curl_close($ch);
  return $out->id;
}

/**
 * Adds metadata to a Zenodo DOI Deposition. Returns the Deposition object.
 *
 * @param int $id
 *   The Deposition identifier.
 * @param object $metadata
 *   The metadata object describing the deposition.
 *
 * @return object
 *   The updated Deposition object.
 */
function tpps_doi_metadata($id, $metadata) {

  $data = new stdClass();
  $data->metadata = $metadata;
  $data_json = json_encode($data);

  $token = variable_get('tpps_zenodo_api_key', NULL);
  $prefix = variable_get('tpps_zenodo_prefix', '');
  $ch = curl_init("https://{$prefix}zenodo.org/api/deposit/depositions/$id?" . http_build_query(array('access_token' => $token)));
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: application/json', 'Content-Length: ' . strlen($data_json)));
  curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'PUT');
  curl_setopt($ch, CURLOPT_POSTFIELDS, $data_json);

  $out = json_decode(curl_exec($ch));
  curl_close($ch);
  return $out;
}

/**
 * Adds files to a Zenodo Deposition from the state of a TPPS Submission form.
 *
 * @param int $id
 *   The identifier for the Deposition.
 * @param array $state
 *   The form_state of the TPPS Submission form.
 */
function tpps_doi_upload_files($id, array &$state) {
  $token = variable_get('tpps_zenodo_api_key', NULL);
  $prefix = variable_get('tpps_zenodo_prefix', '');
  $files = array();

  for ($i = 1; $i <= $state['stats']['species_count']; $i++) {
    $files[] = $state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$i"]['file'];
    if (empty($state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
      break;
    }
  }

  for ($i = 1; $i <= $state['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
    if (isset($state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype'])) {
      $phenotype = &$state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype'];
      if (empty($phenotype['iso-check'])) {
        $files[] = $phenotype['file'];
        if ($phenotype['check']) {
          $files[] = $phenotype['metadata'];
        }
      }
      else {
        $files[] = $phenotype['iso'];
      }
    }

    if (isset($state['saved_values'][TPPS_PAGE_4]["organism-$i"]['genotype'])) {
      $genotype = &$state['saved_values'][TPPS_PAGE_4]["organism-$i"]['genotype'];
      if ($genotype['ref-genome'] == 'manual' or $genotype['ref-genome'] == 'manual2' or $genotype['ref-genome'] == 'url') {
        if ($genotype['tripal_fasta']['file_upload']) {
          $files[] = $genotype['tripal_fasta']['file_upload'];
        }

        if ($genotype['tripal_fasta']['file_upload_existing']) {
          $files[] = $genotype['tripal_fasta']['file_upload_existing'];
        }
      }

      if (!empty($genotype['files']['file-type']['SNPs Genotype Assay'])) {
        $files[] = $genotype['files']['snps-assay'];
      }

      if (!empty($genotype['files']['file-type']['Assay Design']) and $genotype['marker-type']['SNPs']) {
        $files[] = $genotype['files']['assay-design'];
      }

      if (!empty($genotype['files']['file-type']['SSRs/cpSSRs Genotype Spreadsheet'])) {
        $files[] = $genotype['files']['ssrs'];
      }

      if (!empty($genotype['files']['file-type']['Other Marker Genotype Spreadsheet'])) {
        $files[] = $genotype['files']['other'];
      }

      if (!empty($genotype['files']['file-type']['VCF'])) {
        $files[] = $genotype['files']['vcf'];
      }
    }
  }

  foreach ($files as $file) {
    $file = file_load($file);
    if ($file->filesize < 20000000000) {
      $path = drupal_realpath($file->uri);
      $data = array(
        'file' => curl_file_create($path, $file->filemime, $file->filename),
      );

      $ch = curl_init("https://{$prefix}zenodo.org/api/deposit/depositions/$id/files?" . http_build_query(array('access_token' => $token)));
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
      curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-Type: multipart/form-data'));
      curl_setopt($ch, CURLOPT_POST, 1);
      curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
      curl_exec($ch);
      curl_close($ch);
    }
  }
}

/**
 * Publishes a completed Zenodo Deposition. Returns the Deposition object.
 *
 * @param int $id
 *   The identifier for the Deposition.
 *
 * @return object
 *   The updated Deposition object.
 */
function tpps_doi_publish($id) {
  $token = variable_get('tpps_zenodo_api_key', NULL);
  $prefix = variable_get('tpps_zenodo_prefix', '');
  $ch = curl_init("https://{$prefix}zenodo.org/api/deposit/depositions/$id/actions/publish?" . http_build_query(array('access_token' => $token)));
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_setopt($ch, CURLOPT_POSTFIELDS, '');

  $out = json_decode(curl_exec($ch));
  curl_close($ch);
  return $out;
}

/**
 * Loads and returns the Deposition object for an existing Zenodo Deposition.
 *
 * @param int $id
 *   The identifier for the Deposition.
 *
 * @return object
 *   The loaded Deposition object.
 */
function tpps_doi_load($id) {
  $token = variable_get('tpps_zenodo_api_key', NULL);
  $prefix = variable_get('tpps_zenodo_prefix', '');
  $ch = curl_init("https://{$prefix}zenodo.org/api/deposit/depositions/$id?" . http_build_query(array('access_token' => $token)));
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

  $out = json_decode(curl_exec($ch));
  curl_close($ch);
  return $out;
}

/**
 * Formats an author name for Zenodo metadata fields.
 *
 * @param string $name
 *   The name to format.
 *
 * @return string
 *   The formatted name.
 */
function tpps_doi_format_name($name) {
  $parts = explode(' ', $name);
  $first = $parts[0];
  $last = implode(' ', array_slice($parts, 1));
  return "$last, $first";
}
