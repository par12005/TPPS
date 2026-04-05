<?php

/**
 * @file
 * Contains static 'Dryad' class.
 */

/**
 * Manages publication DOI information.
 *
 * To get lists of Publication/Dataset DOI see reports:
 * /admin/reports/tpps/doi/submission/publication.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class Dryad {

  /**
   * Gets DOI Publication information from remote server dryad.
   *
   * Based on code from function tpps_doi_info($doi).
   *
   * @param string $doi
   *   DOI.
   *
   * @return array
   *   Returns response which will be returned back as JSON. Keys:
   *   'error' bool TRUE means validation failed and FALSE that
   */
  public static function getInfo($doi) {
    $key = strtolower($doi);

    // @TODO Minor. Rename cid.
    $cid = 'tpps_get_doi_info';
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    $cache = cache_get($cid, $cache_bin);
    $is_cache_allowed = variable_get('tpps_page_1_cache_ajax_responses', TRUE);

    // @TODO Use 'RestAPI' class.
    if ($reset || !$is_cache_allowed || empty($cache) || !isset($cache->data[$key])) {
      $api_endpoint = variable_get(
        'tpps_page_1_dryad_api_url',
        'http://datadryad.org/api/v2/datasets/'
      );
      $url = $api_endpoint . urlencode('doi:' . $doi);
      // Note: json_decode() could throw an exception on invalid data.
      try {
        $content = file_get_contents($url);
        $data = json_decode($content);
      }
      catch (Exception $e) {
        return t('Caught exception: @message. Try again later.',
          ['@message' => $e->getMessage()]);
      }

      if (empty($data)) {
        // Fake DOI or not found. {"error":"not-found"}.
        $cache->data[$key] = NULL;
      }
      else {
        // Authors.
        foreach ($data->authors as $author) {
          $authors[] = trim($author->firstName . ' ' . $author->lastName);
        }
        $primary_author = $authors[0];
        $secondary_authors = array_slice($authors, 1);
        // Species.
        $species = [];
        foreach ($data->keywords as $keyword) {
          if (preg_match('/^([A-Z][a-z]* [a-z]+)$/', $keyword, $matches)) {
            $species[] = $matches[1];
          }
        }
        $info = [
          'title' => ((substr($data->title, 0, 11) == 'Data from: ')
            ? substr($data->title, 11) : $data->title),
          'primary' => $authors[0],
          'secondary' => $secondary_authors,
          'secondaryNumber' => count($secondary_authors),
          // @TODO [VS] Probably not used. Check!
          'secondaryCheck' => FALSE,
          'year' => explode('-', $data->publicationDate)[0],
          'abstract' => $data->abstract,
          'species' => $species,
          'speciesNumber' => count($species),
        ];
        $cache->data[$key] = $info;
      }
      if (variable_get('tpps_page_1_ajax_debug')) {
        $cache->data[$key]['data'] = $data;
        $cache->data[$key]['url'] = $url;
      }
      cache_set($cid, $cache->data, $cache_bin);
    }
    return $cache->data[$key] ?? [];
  }

}
