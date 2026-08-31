<?php

/**
 * @file
 * Contains 'TppsAjax' class.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * How to add new callback?
 * 1. Create new method. E.g., 'getData'.
 * 2. Make POST-request from JS: /ajax/tpps/getData.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * @TODO Move constant into this class: define('TPPS_AJAX_URL', 'ajax/tpps');
 */

/**
 * Collection of the AJAX callbacks.
 *
 * See tpps_ajax_callback().
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class TppsAjax {

  /**
   * AJAX callback for 'Remember Fieldset Collapsed State' feature.
   *
   * Stores state of given fieldset.
   *
   * @param array $data
   *   POST data.
   *
   * @return bool
   *   Returns TRUE in case of success and FALSE otherwise.
   */
  public static function rememberFieldsetCollapsedState(array $data) {
    global $_SESSION;
    if (
      empty($data['name'])
      && empty($data['form_id'])
      && !isset($data['value'])
    ) {
      return FALSE;
    }
    $form_id = $data['form_id'];
    $name = $data['name'];
    $value = (int) $data['value'];
    $_SESSION[TPPS_RFCS_KEY][$form_id][$name] = $value;
    return TRUE;
  }

  /**
   * Gets NCBI Taxonomy Id.
   *
   * Uses RestApi class and caching.
   *
   * @param array $data
   *   POST data with required 'organism' item which must contain organism name.
   *
   * @return array
   *   Returns array with data to be sent back to browser.
   */
  public static function getOrganismTaxonomyId(array $data) {
    try {
      $organism_name = $data['organism'] ?? NULL;
      if (!empty($organism_name)) {
        $source = 'ncbi';
        $id = NcbiTaxonomy::getId($organism_name);
        if (!empty($id)) {
          $organism_id = tpps_organism($organism_name)->id;
          NcbiTaxonomy::save(
            $organism_id,
            $organism_name,
            $id
          );
        }
        else {
          $source = 'gbif';
          $id = GbifSpeciesApi::getId($organism_name);
          if (!empty($id)) {
            $organism_id = tpps_organism($organism_name)->id;
            GbifSpeciesApi::save(
              $organism_id,
              $organism_name,
              $id
            );
          }
          else {
          }
        }
      }
    }
    catch (Exception $e) {
      // Check synonym.
      $organism_id = OrganismSynonym::search($organism_name);
      $id = NcbiTaxonomy::getId($organism_id);
      if (empty($id)) {
        $id = GbifSpeciesApi::getId($organism_id);
        if (empty($id)) {
          $response['id'] = FALSE;
          $response['success'] = FALSE;
          $response['errors'][] = t("Taxonomy Id not found in NCBI and GBIF databases and by synonym name.");
          return $response;
        }
      }
    }

    $response['id'] = $id ?? FALSE;
    if (empty($response['id'])) {
      $response['success'] = FALSE;
      $response['warnings'][] = t('Organism is missing in NCBI and GBIF databases.');
    }
    else {
      $response['success'] = TRUE;
      $id = check_plain($id);
      if ($source == 'ncbi') {
        // Build link to NCBI Taxonomy Browser page.
        $url = NcbiTaxonomy::getUrl($id);
        $params = [
          'attributes' => [
            'target' => '_blank',
            'title' => t('View using NCBI Taxonomy Browser'),
          ],
        ];
        $response['statuses'][] = t(
          'Organism "@name" found in NCBI database. Taxonomy Id: !id',
          ['@name' => $organism_name, '!id' => l($id, $url, $params)]
        );
      }
      else {
        $url = GbifSpeciesApi::getUrl($id);
        $params = [
          'attributes' => [
            'target' => '_blank',
            'title' => t('View on GBIF site'),
          ],
        ];
        $response['statuses'][] = t(
          'Organism "@name" found in GBIF database. Taxonomy Id: !id',
          ['@name' => $organism_name, '!id' => l($id, $url, $params)]
        );
      }
    }
    return $response;
  }

  /**
   * AJAX callback for 'Get publication data by DOI'.
   *
   * @param array $payload
   *   POST payload. Required keys are: 'doi'.
   *   Keys:
   *     'doi' string.
   *       DOI.
   *     'source' string
   *       Optional. Possible values are:
   *       'all', or data provider (DoiSerp::KEY, DoiAPI::KEY)
   *
   * @return array
   *   Returns response.
   *   Key 'success' will have value TRUE in case of success
   *   and FALSE otherwise.
   *   Other keys:
   *   'success' bool
   *      TRUE if publication data was found and FALSE otherwise.
   *   'publication_data' array
   *       DOI Publication data. See PublicationDoi::TABLE.
   *   'is_complete' bool
   *      TRUE if there is no empty items in Publication data.
   *   'doi' string
   *      DOI.
   *   'statuses' array
   *      List of translated status messages.
   *   'warnings' array
   *      List of translated warning messages.
   *   'errors' array
   *      List of translated error messages.
   */
  public static function getPublicationData(array $payload) {
    $response['success'] = FALSE;
    if (empty($payload)) {
      $response['errors'][] = t('Empty POST requrest.');
    }
    elseif (empty($payload['doi'])) {
      $response['errors'][] = t('Empty DOI.');
    }
    else {
      $response['doi'] = $doi = Doi::searchFirst($payload['doi']);
      $params = ['!doi_link' => Doi::print($doi)];
      if (!Doi::isValid($doi)) {
        $message = 'DOI !doi_link has invalid format. Example DOI: 10.NNNN/dryad.NNN';
        $response['errors'][] = t($message, $params);
      }
      else {
        $response['statuses'][] = t('DOI !doi_link is valid.', $params);
        // Check if DOI in use.
        $doi_check_source = variable_get(
          'tpps_page_1_doi_check_source',
          'public.tpps_submissions'
        );
        if ($doi_check_source == 'public.tpps_submissions') {
          if ($accession = Doi::getAccession($doi)) {
            $response['warnings'][] = t(
              'WARNING: DOI !doi_link is already used by @accession.',
              array_merge($params, ['@accession' => $accession])
            );
          }
          else {
            $response['statuses'][] = t('DOI !doi_link is not in use.', $params);
          }
        }
        else {
          if (Doi::countDbxref($doi) == 0) {
            $response['statuses'][] = t('DOI !doi_link is not in use.', $params);
          }
          else {
            $response['warnings'][] = t(
              'DOI !doi_link is used by another submission.',
              $params
            );
          }
        }
        try {
          $publication_data = PublicationDoi::getCombined($doi);
        }
        catch (Exception $e) {
          // In case of errors just load existing data.
          $publication_data = PublicationDoi::load($doi);
        }
        $response['publication_data'] = $publication_data;
        // When 'is_complete' is FALSE then SERP providers will be used to
        // get more data but SERP providers doesn't have 'species' data.
        $is_complete = PublicationDoi::isComplete($publication_data);
        if (!$is_complete) {
          unset($publication_data['species']);
          $is_complete = PublicationDoi::isComplete($publication_data);
        }
        $response['is_complete'] = $is_complete;
        $response['success'] = TRUE;
      }
    }
    return $response;
  }

  /**
   * AJAX callback for 'Save DOI SERP'.
   *
   * Saves SERP source code into db table per SERP provider.
   * Parses source code using Gemini AI and stores DOI Publication Data
   * into db table 'public.tpps_publication_doi'.
   *
   * @param array $payload
   *   POST payload. Required keys are: 'serp'.
   *   See tpps_ajax_callback().
   *
   * @return array
   *   Returns DOI Publication Data in item with key 'publication_data'.
   *   Other keys and their values:
   *   'doi' (string) - DOI
   *   'success' (bool) - TRUE in case of success and FALSE otherwise.
   *   'source' (string) - source of the DOI Publication data.
   */
  public static function saveSerp(array $payload) {
    if (empty($payload['doi'])) {
      $response['errors'][] = t('Empty DOI.');
    }
    elseif (empty($payload['serp'])) {
      $response['errors'][] = t('Empty SERP.');
    }
    elseif (empty($payload['provider_class_name'])) {
      $response['errors'][] = t('Empty SERP provider class name.');
    }
    else {
      $doi = $payload['doi'];
      $provider_class_name = $payload['provider_class_name'];
      $provider = new $provider_class_name();
      // Skip loading serp from database even if it exists.
      $provider->doi = $doi;
      $provider->serp = base64_decode($payload['serp']);
      if (
        strpos($provider->serp, 'Not found because of proxy error') === FALSE
        && strpos($provider->serp, '404 Not Found') === FALSE
      ) {
        // Save the SERP page source code.
        $provider->save();
        // Parsed data will be stored into PublicationDoi::TABLE.
        $provider->parse();

        $response['doi'] = $doi;
        $response['provider_class_name'] = $payload['provider_class_name'];
        $response['publication_data'] = PublicationDoi::load($doi, NULL, TRUE);
      }
      else {
        $response['errors'][] = t('Proxy error.');
      }
    }
    return $response;
  }

}
