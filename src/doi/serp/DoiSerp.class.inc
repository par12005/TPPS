<?php

/**
 * @file
 * Contains 'DoiSerp' class.
 */

/**
 * Gets publication info by DOI form Google Scholar.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * How to extend class?
 *
 * 1. Come up with unique name. It will be static::KEY.
 * 2. Constant TABLE must have 'tpps_' prefix
 *    and usually has KEY constant as suffix.
 * 3. Create Drupal variables:
 *    'tpps_doi_' . static::KEY . '_serp_path',
 *    'tpps_doi_' . static::KEY . '_use_ip',
 *    'tpps_doi_' . static::KEY . '_delay',
 *    'tpps_doi_' . static::KEY . '_debug_mode'.
 * 4. Implement getQuery() method.
 * 5. Register class in *.install file.
 * 6. Add iframe to /tpps/doi/lookup page.
 * 7. Update JS: /js/doi_lookup.js
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
abstract class DoiSerp {

  // Unique key used to get Drupal variables and etc.
  // To get key by class name: $name = constant($class_name . '::KEY');
  // WARNING:
  // In DB table 'public.tpps_publication_doi.source' used class name
  // (not class's KEY constant).
  const KEY = '';
  // Name of the DB table where SERP is stored.
  const TABLE = '';
  // Default value of the delay between requests.
  // Value 10-60 seconds is suggested over the internet to avoid IP address ban.
  const DEFAULT_DELAY = 5;
  // Relative path to the SERP page.
  // Used to build URL for iframe.
  const SERP_PATH = '';
  // Name of the GET-query parameter which holds DOI.
  const QUERY_PARAM = '';
  // Domain name (without schema because HTTPS will always be used).
  const HOST_NAME = '';
  // IPv4 address.
  const HOST_IP = '';
  // Token to detect page without publication data.
  const NOT_FOUND_TOKEN = '';

  /**
   * Indicates if DOI SERP provider is allowed for scraping.
   *
   * Default value is stored in variable:
   *   'tpps_doi_' . static::KEY . '_scrape_allowed'.
   * To change in run-time (overwrite default value) use:
   *   $provider->scrapeAllowed = FALSE;
   *
   * @var bool
   */
  protected $scrapeAllowed = TRUE;

  /**
   * Indicates if 'Debug Mode' is enabled/disabled.
   *
   * To change in run-time: $provider->debugMode = TRUE;
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
   * SERP hash.
   *
   * Stored in tpps_google_scholar.md5 as varchar 32 byte.
   *
   * @var string
   */
  protected $md5;

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
    if (!empty(static::TABLE) && !db_table_exists(static::TABLE)) {
      throw new Exception('DB table "' . static::TABLE . '" not found.');
    }
    if (!empty($doi)) {
      $this->doi = $doi;
      $this->load();
    }
    $this->scrapeAllowed = variable_get(
      'tpps_doi_' . static::KEY . '_scrape_allowed',
      TRUE
    );
    $this->delay = variable_get(
      'tpps_doi_' . static::KEY . '_delay',
      static::DEFAULT_DELAY
    );
    $this->debugMode = variable_get(
      'tpps_doi_' . static::KEY . '_debug_mode',
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
    // Get data from DB.
    $result = db_select(static::TABLE, 't')
      ->fields('t')
      ->condition('doi', $this->doi)
      // Load latest record.
      ->range(0, 1)
      ->orderBy('created', 'DESC')
      ->execute()
      ->fetchAssoc();
    $this->isLoaded = TRUE;
    // Set properties.
    if ($result) {
      $this->created = $result['created'] ?? 0;
      $this->serp = $result['serp'] ?? '';
      $this->md5 = $result['md5'] ?? '';
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
   * @return bool
   *   Returns TRUE if new record was created and FALSE otherwise (the same
   *   SERP already exists in database).
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
    $this->md5 = md5($this->serp);
    $this->created = REQUEST_TIME;

    $is_exists = (bool) db_select(static::TABLE, 't')
      ->fields('t', ['doi'])
      ->condition('doi', $this->doi)
      ->condition('md5', $this->md5)
      ->range(0, 1)
      ->execute()
      ->fetchField();
    if (!$is_exists) {
      db_insert(static::TABLE)->fields([
        'doi' => $this->doi,
        'created' => $this->created,
        'serp' => $this->serp,
        'md5' => $this->md5,
      ])->execute();
      return TRUE;
    }
    return FALSE;
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
      $this->getEndpointUrl(),
      ['query' => $this->getQuery()]
    );
    return $url;
  }

  /**
   * Scraps SERP to get publication info.
   *
   * Publication data will be saved using PublicationDOI::save().
   *
   * @return array
   *   Returns parsed data.
   *
   * @throws Exception
   *   When $doi is empty.
   *   When data loaded from DB but $serp is empty.
   */
  public function parse() {
    if (empty($this->doi)) {
      throw new Exception("Can't create URL because of the empty DOI.");
    }
    if (!$this->isLoaded) {
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

        // @TODO Get already known data and send to Gemini.
        $gemini->systemInstruction = variable_get(
          'tpps_doi_lookup_system_instruction',
          '
          Get DOI publication information from given source code of the page.
          Journal must not have year.
          Authors must be separated by commas (not semicolons).
          Use string "NA" for empty values.
          '
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
      $fields = json_decode($gemini->answer, 1);
      if ($fields === NULL || $fields === FALSE) {
        // An error occurred during decoding.
        $error_code = json_last_error();
        $error_message = json_last_error_msg();
        throw new Exception("JSON decoding error: [{$error_code}] {$error_message}");
      }
      $fields['source'] = static::class;
      $fields['doi'] = $this->doi;
    }
    else {
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Custom parser.
      // @TODO Minor. Implement custom parser.
      //
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    }

    // Store result (if possible).
    if (class_exists('PublicationDOI', $autoload = TRUE)) {
      if (PublicationDOI::isMissing($fields['doi'], $fields['source'])) {
        $result = PublicationDOI::save($fields);
        if ($result) {
          // Successfully saved into db.
        }
        else {
          watchdog('tpps_google_scholar',
            "Parsed DOI publication data wasn't saved into database. <pre>@dump</pre>",
            ['@dump' => print_r($fields, 1)],
            'error'
          );
        }
      }
      else {
        // Record exists.
        $db_data = PublicationDOI::load($fields['doi'], $fields['source']);
        if (PublicationDOI::isComplete($db_data)) {
          // Publication Data is complete. No need to update it.
        }
        else {
          // @todo Minor. Compare recently obtained data and data from DB.
          PublicationDOI::replace($fields);
        }
      }
    }
    else {
      watchdog('tpps_google_scholar', "Class PublicationDOI not found", [], 'error');
    }
    return $fields;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Custom methods.

  /**
   * Gets query (url params) to build URL to get SERP.
   *
   * @return array
   *   Returns query for url().
   */
  public function getQuery() {
    return [];
  }

  /**
   * Gets DB table schema.
   *
   * Table to store Google Scholar SERP pages.
   * Used in hook_install().
   * Table name: static::TABLE.
   * Table must have 'doi' and 'created' fields.
   *
   * Usage example:
   *   $field_list = array_keys(static::getSchema()['fields']);
   *   unset($field_list['source']);
   *
   * @return array
   *   Returns db table schema.
   */
  public static function getSchema() {
    return [
      'description' => 'Stores SERP of the ' . static::KEY . ' website.',
      'fields' => [
        'doi' => [
          'description' => 'DOI (Digital Object Identifier).',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'created' => [
          'type' => 'int',
          'not null' => TRUE,
          'default' => 0,
          'description' => 'Unix timestamp of when the entry was created.',
        ],
        'serp' => [
          'description' => 'SERP (Search Engine Results Page).',
          'type' => 'text',
          'size' => 'medium',
          'not null' => TRUE,
        ],
        'md5' => [
          'description' => 'Hash of the serp string to check uniqueness.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
      ],
      'unique keys' => [
        'doi_md5' => ['doi', 'md5'],
      ],
    ];
  }

  /**
   * Gets Endopoint URL.
   *
   * @return string
   *   Returns Endpoint URL.
   *   No trailing slash.
   */
  public function getEndpointUrl() {
    $host = static::HOST_NAME;
    if (variable_get('tpps_doi_' . static::KEY . '_use_ip', FALSE)) {
      $host = static::HOST_IP;
    }
    $path = variable_get('tpps_doi_' . static::KEY . '_serp_path', static::SERP_PATH);
    return 'https://' . $host . '/' . $path;
  }

  /**
   * Gets class name.
   *
   * @return string
   *   Returns class name.
   */
  public function getClassName() {
    return static::class;
  }

  /**
   * Gets string to detect that page has no publication data.
   *
   * When DOI not found provider shows suggestions or empty string which
   * shouldn't be processed.
   *
   * @return string
   *   Returns string which allows to detect missing publication data at page.
   */
  public function getNotFoundString() {
    return variable_get('tpps_doi_' . static::KEY . '_not_found_token', '');
  }

}
