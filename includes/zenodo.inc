<?php

/**
 * @file
 * Defines functions to create and populate Zenodo DOIs.
 */

define('TPPS_20GB', 20000000000);

/**
 * Generates URL for different API endpoints.
 *
 * @param string $endpoint
 *   Zenodo Endpoint.
 *
 * @return string
 *   Returns absolute URL with access token.
 */
function tpps_zenodo_build_url($endpoint = '') {
  // @TODO Statically cache this URL.
  $token = variable_get('tpps_zenodo_api_key', NULL);
  $prefix = variable_get('tpps_zenodo_prefix', '');
  // @TODO Use url().
  return "https://{$prefix}zenodo.org/api/deposit/depositions"
    . ($endpoint ? "/$endpoint" : "")
    . "?" . http_build_query(['access_token' => $token]);
}

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
  $submission = new Submission($accession);
  $state = $submission->state;
  $page1_values = $state['saved_values'][TPPS_PAGE_1] ?? NULL;
  $page2_values = $state['saved_values'][TPPS_PAGE_2] ?? NULL;

  $id = tpps_doi_create();
  $creators = array(
    (object) array(
      'name' => tpps_doi_format_name($state['saved_values'][TPPS_PAGE_1]['primaryAuthor']),
      // [VS] Organization field was removed so I just left 'affiliation' with
      // empty value because I'm not sure if this field is required.
      'affiliation' => NULL,
      // [/VS]
    ),
  );
  $secondary_author_number = $page1_values['publication']['secondaryAuthors']['number'];
  for ($i = 1; $i <= $secondary_author_number; $i++) {
    $creators[] = (object) [
      'name' => tpps_doi_format_name($page1_values['publication']['secondaryAuthors'][$i]),
    ];
  }

  $keywords = [];
  for ($i = 1; $i <= $page1_values['organism']['number']; $i++) {
    $keywords[] = $page1_values['organism'][$i]['name'];
  }

  module_load_include('inc', 'tpps', 'includes/form');
  $keywords[] = tpps_form_get_experimental_design_options($page2_values['study_type']);

  $metadata = (object) array(
    'title' => $page1_values['publication']['title'],
    'upload_type' => 'dataset',
    'description' => $page1_values['publication']['abstract'],
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
  $ch = curl_init(tpps_zenodo_build_url());
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

  $ch = curl_init(tpps_zenodo_build_url($id));
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_HTTPHEADER, array(
    'Content-Type: application/json',
    'Content-Length: ' . strlen($data_json),
  ));
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
  $files = [];
  for ($i = 1; $i <= $state['stats']['species_count']; $i++) {
    $files[] = $state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$i"]['file'];
    if (empty($state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
      break;
    }
  }

  for ($i = 1; $i <= $state['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
    if (isset($state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype'])) {
      $phenotype = &$state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype'];
      if (!empty($phenotype['normal-check'])) {
        $files[] = $phenotype['file'];
        if ($phenotype['check']) {
          $files[] = $phenotype['metadata'];
        }
      }

      if (!empty($phenotype['iso-check'])) {
        $files[] = $phenotype['iso'];
      }
    }

    // Genotype.
    if (isset($state['saved_values'][TPPS_PAGE_4]["organism-$i"]['genotype'])) {
      $genotype = &$state['saved_values'][TPPS_PAGE_4]["organism-$i"]['genotype'];
      if (in_array($genotype['ref-genome'], ['manual', 'manual2', 'url'])) {
        $files[] = $genotype['tripal_fasta']['file_upload'] ?? NULL;
        $files[] = $genotype['tripal_fasta']['file_upload_existing'] ?? NULL;
      }
      // @TODO Could we skip all this extra checks and just collect files?
      // @TODO Ask curation team what to do with this.
      // WARNING: Submission State (not Shared State, v.1).
      $file_type = $genotype[$snps_fieldset]['file-type'] ?? NULL;
      $snps_fieldset = 'SNPs';
      $ssrs_fieldset = 'ssrs_cpssrs';
      if ($genotype['does_study_include_snp_data'] == 'yes') {
        if ($file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE) {
          $files[] = $genotype[$snps_fieldset]['snps-assay'] ?? NULL;
          $files[] = $genotype[$snps_fieldset]['assay-design'] ?? NULL;
        }
        elseif ($file_type == TPPS_GENOTYPING_FILE_TYPE_VCF) {
          $files[] = $genotype[$snps_fieldset]['vcf'] ?? NULL;
        }
      }
      if ($genotype['does_study_include_ssr_cpssr_data'] == 'yes') {
        $files[] = $genotype[$ssrs_fieldset]['ssrs'] ?? NULL;
        // @TODO What about cpssrs?
        // $files[] = $genotype[$ssrs_fieldset]['cpssrs'] ?? NULL;
      }
      if ($genotype['does_study_include_other_genotypic_data'] == 'yes') {
        $files[] = $genotype['other']['other'] ?? NULL;
      }
    }
  }
  $files = array_filter($files);

  foreach ($files as $fid) {
    if ($fid && $file = tpps_file_load($fid) && $file->filesize < TPPS_20GB) {
      $path = tpps_realpath($file->uri);
      $data = [
        'file' => curl_file_create($path, $file->filemime, $file->filename),
      ];
      $ch = curl_init(tpps_zenodo_build_url("$id/files"));
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
  $ch = curl_init(tpps_zenodo_build_url("$id/actions/publish"));
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
  $ch = curl_init(tpps_zenodo_build_url($id));
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
