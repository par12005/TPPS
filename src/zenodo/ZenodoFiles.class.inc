<?php

/**
 * @file
 * Contains 'ZenodoFiles' class.
 */

/**
 * Zenodo API for Deposition Files.
 *
 * See: https://developers.zenodo.org/#deposition-files.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class ZenodoFiles extends Zenodo {

  // Prefix of the API URL.
  const BASE_PATH = '/api/deposit/depositions';

  // Limit of the single file published using Zenodo service.
  // 20Gb.
  const FILE_SIZE_LIMIT = 20000000000;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Custom methods.

  /**
   * Generates URL for Zenodo API Endpoints.
   *
   * @param string $endpoint
   *   Part of the URL after 'https://zenodo.org/api/deposit/depositions'.
   *
   * @return string
   *   Returns absolute URL with access token.
   */
  public static function getUrl($endpoint = '') {
    $host = self::LIVE_HOST;
    if (variable_get('tpps_zenodo_debug_mode')) {
      $host = self::DEBUG_HOST;
    }
    return url(
      $host . '/api/deposit/depositions' . ($endpoint ?? "/$endpoint"),
      [
        // Note: access token could be sent in headers.
        'query' => ['access_token' => variable_get('tpps_zenodo_api_key')],
        'absolute' => TRUE,
        'https' => TRUE,
      ]
    );
  }




  /**
   * Adds files to a Zenodo Deposition from the state of a TPPS Submission form.
   *
   * @param int $deposit_id
   *   Zenodo Deposit Id.
   * @param array $state
   *   The form_state of the TPPS Submission form.
   */
  public static function upload($deposit_id, array &$state) {
    $debug_mode = 0;

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
        $snps_fieldset = 'SNPs';
        $ssrs_fieldset = 'ssrs_cpssrs';
        $file_type = $genotype[$snps_fieldset]['file-type'] ?? NULL;
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

    if (!empty($debug_mode)) {
      dpm(print_r($files, 1));
      dpm(print_r($genotype, 1));
      return;
    }

    $token = variable_get('tpps_zenodo_api_key', NULL);
    $upload_file_count = 0;
    foreach ($files as $fid) {
      if (
        !empty($file = tpps_file_load($fid))
        && $file->filesize < self::FILE_SIZE_LIMIT
      ) {
        if (!is_object($file)) {
          continue;
        }
        dpm('Uploading file:');
        dpm('FID:' . $fid);
        dpm($file);
        $path = tpps_realpath($file->uri);
        $data = [
          'file' => curl_file_create($path, $file->filemime, $file->filename),
          'token' => $token
        ];
        $out = RestAPI::post($
          static::getUrl("$deposit_id/files"),
          $data,
          $header
        )
        $upload_file_count++;
      }
    }
    if ($upload_file_count > 0) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

}
