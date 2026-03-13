<?php

/**
 * @file
 * Contains 'GoogleScholar' class.
 *
 * What this class does?
 * 1. Saves/loads Google Scholar SERP for certain DOI into DB table:
 * 'tpps_google_scholar'.
 * 2. This class DO NOT requests SERP directly because those requests will be
 * blocked by Google Scholar.
 * 3. Manages delay between requests.
 * 4. Manages parsing. Data will be stored in PublicationDOI::TABLE table.
 *
 * Workaround.
 *
 * Direct requests to Google Scholar will be blocked.
 * Setup CORS Anywhere proxy server which cuts some headers from the Google
 * Scholar GET-request of the search result page (SERP).
 * Use client's browser to get Google Scholar response in iframe.
 * Note: curl requests looks like a a bot/scraper requests and will be blocked.
 * Request from client's browser looks more natural (has cookies and etc.).
 * Content from client's browser will be sent to backend and saved by this class.
 *
 *
 * Usage:
 *  // Save SERP.
 *  $gs = new GoogleScholar();
 *  $gs->doi = $doi;
 *  $gs->serp = $content;
 *  $gs->save();
 *  // Load SERP.
 *  $gs = new GoogleScholar($doi);
 *  dpm($gs->serp);
 *
 *
 * :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * @todo Request Google scholar front page at first step to emulate human's
 * behavior. Note: form uses hidden fields which could be changed anytime.
 * @todo Currently every call of save() will create new record in DB table.
 * This means that table size will grow very quickly but it means we have a
 * history. Find a way to remove outdated data.
 */

/**
 * Gets publication info by DOI form Google Scholar.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class GoogleScholar {

  // Name of the DB table where SERP is stored.
  const TABLE = 'tpps_google_scholar';
  // Default value of the delay between requests.
  // Value 10-60 seconds is suggested over the internet to avoid IP address ban.
  const DEFAULT_DELAY = 2;
  const SERP_URL = 'https://scholar.google.com/scholar';

  /**
   * Indicates if 'Debug Mode' is enabled/disabled.
   *
   * To change in run-time: $gs->debugMode = TRUE;
   *
   * @var bool
   */
  protected $debugMode = TRUE;

  /**
   * DOI (Digital Object Identifier).
   *
   * Stored in tpps_google_scholar.doi as string.
   * Format example: 10.1002/ece3.4216, 10.5061/dryad.1mt1h.
   *
   * @var string
   */
  protected $doi;

  /**
   * Indicates that data was loaded from DB.
   *
   * @var bool
   */
  protected $isLoaded = FALSE;

  /**
   * Indicates that SERP data was found in DB.
   *
   * When there is no record for given DOI than after calling load()
   * $isLoaded will be TRUE but $isExists will be FALSE.
   * Property $isLoaded used to avoid multiple requests to DB (instead
   * of static caching).
   *
   * @var bool
   */
  protected $isExists = FALSE;


  /**
   * Unix timestamp of the moment when SERP was obtained and saved into DB.
   *
   * Stored in tpps_google_scholar.created as int.
   *
   * @var int
   */
  protected $created;

  /**
   * Google Scholar SERP.
   *
   * Stored in tpps_google_scholar.serp as text.
   *
   * @var string
   */
  protected $serp;

  /**
   * Unix timestamp of the last request stored in table 'tpps_google_scholar'.
   *
   * @var int
   */
  protected $lastRequestTime;

  /**
   * Delay in seconds between requests to avoid IP ban.
   *
   * Could be changed at Google Scholar settings page or directly:
   * $gs = new GoogleScholar($doi);
   * $gs->set('delay', 30);
   *
   * @var int
   */
  protected $delay;

  /**
   * Class constructor.
   *
   * @param string $doi
   *   DOI (Digital Object Identifier).
   *
   * @throws Exception
   *   When db table which stores Google Scholar SERP doen't exists.
   */
  public function __construct(string $doi = NULL) {
    if (!db_table_exists(self::TABLE)) {
      throw new Exception('DB table ' . self::TABLE . ' not found.');
    }
    if (!empty($doi)) {
      $this->doi = $doi;
      $this->load();
    }
    $this->delay = variable_get(
      'tpps_google_scholar_delay',
      static::DEFAULT_DELAY
    );
    $this->debugMode = variable_get(
      'tpps_google_scholar_debug_mode',
      FALSE
    );
    return $this;
  }

  /**
   * Magic setter.
   *
   * @param string $name
   *   Class property name.
   * @param mixed $value
   *   Value to be set.
   *
   * @throws Exception
   *   When requested non-existing property.
   */
  public function __set($name, $value) {
    // Allow to overwrite magic with custom setter.
    $func = tpps_snake_to_camel_case('set_' . $name);
    if (method_exists($this, $func)) {
      $this->$func($value);
    }
    else {
      if (property_exists($this, $name)) {
        $this->$name = $value;
      }
      else {
        throw new Exception("Inexistent property: $name");
      }
    }
    return $this;
  }

  /**
   * Magic getter.
   *
   * @param string $name
   *   Class property name.
   *
   * @return mixed
   *   Returns property value.
   *
   * @throws Exception
   *   When requested non-existing property.
   */
  public function &__get($name) {
    // Allow to overwrite magic with custom getter.
    $func = tpps_snake_to_camel_case('get_' . $name);
    if (method_exists($this, $func)) {
      return $this->$func();
    }
    else {
      if (property_exists($this, $name)) {
        return $this->$name;
      }
      else {
        throw new Exception("Inexistent property: $name");
      }
    }
    return $this;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Loads SERP from database.
   *
   * @throws Exception
   *   When $this->doi is empty.
   */
  public function load() {
    if (empty($this->doi)) {
      throw new Exception(t('Study accession is empty.'));
    }
    $this->isLoaded = TRUE;
    // Get data from DB.
    $result = db_select(self::TABLE, 't')
      ->fields('t')
      ->condition('doi', $this->doi)
      // Load latest record.
      ->range(0, 1)
      ->orderBy('created', 'DESC')
      ->execute()
      ->fetchAssoc();
    // Set properties.
    if ($result) {
      $this->created = $result['created'] ?? 0;
      $this->serp = $result['serp'] ?? '';
      $this->isExists = !empty($this->serp);
    }
    else {
      $this->isExists = FALSE;
    }

    return $this;
  }

  /**
   * Saves SERP into database.
   *
   * @throws Exception
   *   When $this->doi or $this->serp is empty.
   */
  public function save() {
    if (empty($this->doi)) {
      throw new Exception('Empty DOI.');
    }
    if (empty($this->serp)) {
      throw new Exception('Empty SERP.');
    }
    db_insert(self::TABLE)->fields([
      'doi' => $this->doi,
      'created' => REQUEST_TIME,
      'serp' => $this->serp,
    ])->execute();
    $this->created = REQUEST_TIME;
  }

  /**
   * Gets query (url params) to build URL to get SERP.
   *
   * @return array
   *   Returns query for url().
   */
  public function getQuery() {
    return [
      'hl' => 'en',
      'as_sdt' => '0,5',
      'q' => urlencode(check_plain($this->doi)),
      'btnG' => NULL,
    ];
  }

  /**
   * Gets Endopoint URL.
   *
   * @return string
   *   Returns Endpoint URL.
   */
  public function getEndpointUrl() {
    return variable_get(
      'tpps_doi_google_scholar_serp_url',
      GoogleScholar::SERP_URL
    );
  }

  /**
   * Builds URL to get SERP.
   *
   * @return string
   *   Returns escaped URL.
   */
  public function buildUrl() {
    if (empty($this->doi)) {
      throw new Exception("Can't create URL because of the empty DOI.");
    }
    $url = url(
      $this->getEndpointURL(),
      ['query' => $this->getQuery()]
    );
    return $url;
  }

  /**
   * Scraps SERP to get publication info.
   *
   * @throws Exception
   *   When $doi is empty.
   *   When data loaded from DB but $serp is empty.
   */
  public function parse() {
    if (empty($this->doi)) {
      throw new Exception("Can't create URL because of the empty DOI.");
    }
    if (!$this->isLoaded()) {
      $this->load();
    }
    if (empty($this->serp)) {
      // @TODO Call $this->request().
      throw new Exception("Empty SERP.");
    }

    $use_gemini = TRUE;
    if ($use_gemini) {
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

      $gemini = new Gemini();
      if (!empty($values['tpps_doi_lookup_system_instruction'])) {
        $gemini->systemInstruction = variable_get(
          'tpps_doi_lookup_system_instruction',
          'Get DOI publication information for given page source.'
        );
      }
      // Answer in JSON format.
      $gemini->generationConfig = [
        "responseMimeType" => "application/json",
        "responseJsonSchema" => [
          "type" => "object",
          "properties" => [
            "doi" => (object) ["type" => "string"],
            "title" => (object) ["type" => "string"],
            "journal" => (object) ["type" => "string"],
            "date" => (object) ["type" => "string"],
            "authors" => (object) ["type" => "string"],
            "abstract" => (object) ["type" => "string"],
          ],
          "required" => ["title", "authors", "date", "abstract", "journal"],
        ],
      ];

      // @TODO Try to allow internet search to get better results.
      // Note:
      // Disabled because requiests failed and doesn't actually required.
      //$gemini->tools = [["google_search" => (object) []]];

      $gemini->ask($this->serp);
      $fields = json_decode($gemini->answer, TRUE);

      if ($fields === NULL || $fields === FALSE) {
        // An error occurred during decoding.
        $error_code = json_last_error();
        $error_message = json_last_error_msg();
        throw new Exception("JSON decoding error: [{$error_code}] {$error_message}");
      }

      // Store to DB.
      $fields['source'] = 'gemini';
      // @TODO Remove this debug code.
      $fields['doi'] = $this->doi ?? '10.1002\/ece3.4216';
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    }
    else {
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Custom parser.
      // @TODO Minor. Implement custom parser.
      //
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    }

    // Store result.
    PublicationDOI::save($fields);
  }

}
