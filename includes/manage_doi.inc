<?php

/**
 * @file
 * Contains DOI-related functions for TPPS.
 */

/**
 * Shows DOI of a TPPS submission.
 *
 * If no DOI exists for the submission, one is automatically generated.
 *
 * @param string $accession
 *   The accession number of the TPPS submission.
 *
 * @return string
 *   The HTML for the DOI page.
 */
function tpps_manage_doi($accession) {
  $output = "";

  $form_state = tpps_load_submission($accession);

  $token = variable_get('tpps_zenodo_api_key', NULL);
  if (!empty($token) and empty($form_state['zenodo_id'])) {
    $form_state['zenodo_id'] = tpps_generate_doi($accession)->id;
    $doi_object = tpps_doi_publish($form_state['zenodo_id']);
    tpps_update_submission($form_state);
    $output .= "<h4>DOI successfully published!</h4>";
  }
  elseif (!empty($token) and !empty($form_state['zenodo_id'])) {
    $doi_object = tpps_doi_load($form_state['zenodo_id']);
  }

  if (empty($token) or empty($doi_object) or (isset($doi_object->status) and $doi_object->status != 200)) {
    $output .= "<h4>Unable to load Zenodo DOI. There is likely a problem with the Zenodo API key. Please contact your system administrator.</h4>";
  }
  else {
    $doi = $doi_object->doi;
    $url = $doi_object->doi_url;
    $badge = $doi_object->links->badge;
    $output .= "DOI: <a target=\"blank\" href=\"$url\">$doi</a><br>"
    . "DOI Badge: <a target=\"blank\" href=\"$badge\"><img src=\"$badge\"></a><br>";
  }

  return render($output);
}

/**
 * Loads info about a Dryad DOI.
 *
 * @param string $doi
 *   The doi being loaded.
 *
 * @return mixed
 *   The retrieved information, or FALSE on failure.
 */
function tpps_doi_info($doi) {
  if (empty($doi)) {
    return FALSE;
  }

  if (!preg_match(tpps_doi_regex(), $doi)) {
    drupal_set_message(t('Invalid DOI format. Example DOI: 10.1111/dryad.111'), 'warning');
    return FALSE;
  }

  $query = db_select('chado.dbxref', 'd')
    ->condition('accession', $doi)
    ->execute();
  if (count($query->fetchAll()) > 0) {
    drupal_set_message(t('This DOI is already in use for another submission.'), 'warning');
    return FALSE;
  }

  $result = array();
  $url = "http://datadryad.org/api/v2/datasets/doi%3A" . urlencode($doi);
  $response_json_data = json_decode(file_get_contents($url));

  if (!empty($response_json_data)) {
    $result['title'] = $response_json_data->title;
    if (substr($result['title'], 0, 11) == 'Data from: ') {
      $result['title'] = substr($result['title'], 11);
    }

    $authors = $response_json_data->authors;
    $result['primary'] = trim(implode(" ", array($authors[0]->firstName, $authors[0]->lastName)));
    $result['second'] = array();
    $authors = array_slice($authors, 1);
    foreach ($authors as $author) {
      $result['second'][] = trim(implode(" ", array($author->firstName, $author->lastName)));
    }
    $result['second']['number'] = count($authors);
    $result['second']['check'] = FALSE;

    $result['year'] = explode('-', $response_json_data->publicationDate)[0];

    $result['abstract'] = $response_json_data->abstract;

    $result['species'] = array();
    foreach ($response_json_data->keywords as $keyword) {
      if (preg_match('/^([A-Z][a-z]* [a-z]+)$/', $keyword, $matches)) {
        $result['species'][] = $matches[1];
      }
    }
  }
  return $result;
}

/**
 * Returns expression for matching DOIs.
 *
 * @return string
 *   The regular expression pattern for DOIs.
 */
function tpps_doi_regex() {
  return '/^10\.\d{4,9}[\-._;()\/:A-Za-z0-9]+$/';
}
