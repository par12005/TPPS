<?php

/**
 * @file
 * Contains DOI-related functions for TPPS.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/manage_doi');
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

/**
 * Searches for DOI in already submitted studies.
 *
 * Note: Since caching enabled if multiple studies uses the same DOI (which
 * usually possible only for test studies) then latest study accession will
 * be used. Before first match was used.
 *
 * Total execution time:
 * Getting data from cache: 0.00034999847412109 sec
 * Empty cache or reset:    0.96340107917786 sec
 *
 * @param string $doi
 *   DOI value.
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 *
 * @return string
 *   Returns accession in format 'TGDRxxx'.
 *   Returns FALSE if DOI wasn't found.
 *
 * @TODO Minor. Search for exact doi value instead of getting all the data.
 * See tpps_search_used_doi_NEW_VERSION().
 */
function tpps_search_used_doi($doi, $reset = FALSE) {
  $cid = __FUNCTION__;
  $cache = cache_get($cid);
  if ($reset || empty($cache)) {
    $all_submissions = chado_query("SELECT * FROM public.tpps_submission;");
    foreach ($all_submissions as $item) {
      $form_state = unserialize($item->submission_state);
      $submission_doi = strtolower($form_state['saved_values']['1']['doi'] ?? NULL);
      if ($submission_doi) {
        $cache->data[$submission_doi] = $item->accession ?? NULL;
      }
    }
    cache_set($cid, $cache->data);
  }
  return $cache->data[strtolower($doi)] ?? NULL;
}

/**
 * Searches for DOI in already submitted studies.
 *
 * WARNING: This code doesn't work!
 *
 * @param string $doi
 *   DOI value.
 *
 * @return string
 *   Returns accession in format 'TGDRxxx'.
 *   Returns FALSE if DOI wasn't found.
 */
function tpps_search_used_doi_NEW_VERSION($doi) {

  // WARNING: This code doesn't work!
  return;

  // The goal was to use LIKE operator to search of substring but
  // submission's state is stored serialized in 'bytea' column.
  //
  // Problems:
  // ==ILIKE on 'bytea' gives an error ==
  //
  //   SELECT ts.submission_state FROM tpps_submission ts WHERE submission_state ILIKE '%0965%'
  //
  // Result:
  //   ERROR:  operator does not exist: bytea ~~* unknown
  //   LINE 5:         AND submission_state ILIKE '%0965%') AS sub
  //                                        ^
  //   HINT:  No operator matches the given name and argument types. You might need to add explicit type casts.
  //
  // == But LIKE works! ==
  //
  //   SELECT ts.submission_state FROM tpps_submission ts WHERE submission_state LIKE '%0965%'
  //
  // Result: no errors and I'm getting 4 pages of results.
  //
  // == use of lower() function ==
  //
  //   SELECT ts.submission_state FROM tpps_submission ts WHERE LOWER(submission_state) LIKE '%0965%'
  //
  // Result:
  //
  //   ERROR:  function lower(bytea) does not exist
  //   LINE 1: ...ts.submission_state FROM tpps_submission ts WHERE LOWER(subm...
  //                                                                ^
  //   HINT:  No function matches the given name and argument types. You might need to add explicit type casts.
  // Seems lower() and ILIKE do not work with 'bytea' datatype....

  // $time_start = microtime(1);

  // We don't use serialize(['doi' => $doi]) because it also wraps needed
  // string with "a:1{}" and do not escape qoutes. So it's easier to build
  // string manually.
  foreach (['doi', check_plain(trim($doi))] as $value) {
    @$needle .= 's:' . strlen($value) . ':&quot;' . $value . '&quot;';
    //@$needle .= 's:' . strlen($value) . ':"' . $value . '";';
  }

  $needle = '0965';

  // Data type 'bytea' require some escaping for zeros.
  // https://www.postgresql.org/docs/current/datatype-binary.html
  //$needle = str_replace('0', '\000', db_like($needle));

  dpm(db_like($needle));

  // ILIKE is not supported by MySQL and do not work with 'bytea' data type in Postgres.
  // https://github.com/interchange/TableEditor/issues/64
  if (1) {
    // Check the tpps_submissionos.
    // @TODO Search for doi value instead of getting all the data.
    // It's a huge amount of data.
    $sql = "SELECT ts.submission_state
      FROM {tpps_submission} ts
      WHERE
        submission_state IS NOT NULL
        AND lower(submission_state) LIKE '%" . $needle . "%'
  ";
    dpm($sql);
    // LIMIT 1 OFFSET 0
    $result = db_query($sql);
  }
  else {
    $result = db_select('tpps_submission', 'ts')
      ->addTag('debug')
      ->fields('ts', ['submission_state'])
      ->condition('submission_state', '%' . $needle . '%', 'LIKE')
      ->execute();
  }
  $submission = $result->fetchField();

  dpm(print_r($submission, 1));
  return;

  //if (!empty($submission)) {
  //  $submission = unserialize($submission);
  //  if (strtolower($doi) == ddstrtolower($submission_state['saved_values']['1']['doi'])) {
  //    $accession = $submission->accession;
  //    break;
  //  }
  //}

  // Old:
  // 10.1111/dryad.111
  // Total execution time: 0.60140991210938 sec
  // 10.25338/B8864J
  // Total execution time: 0.36915397644043 sec
  // $accession .= '<br/> Total execution time: ' . (microtime(1) - $time_start) . ' sec';

  return ($accession ?? FALSE);
}
