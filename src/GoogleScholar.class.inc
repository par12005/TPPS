<?php

/**
 * @file
 * Contains 'GoogleScholar' class.
 *
 * Usage:
 *  $gs = new GoogleScholar($doi);
 *  $gs->request();
 *  $gs->getJornal();
 *  $gs->update();
 *
 * Workflow
 * 1. Set DOI.
 * 2. Get response using RestAPI.
 * 3. Store in DB.
 * 4. Parse.
 *
 * n. Update if stored pages is outdated.
 *
 * @TODO Imitate form https://scholar.google.com/ submit (with cookies).
 *
 * :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Core Concepts:
 * Respect robots.txt: Be aware that scraping is against Google Scholar's Terms of Service, and you should check scholar.google.com/robots.txt before starting .
 * Use Proxies: To avoid IP bans, you'll need to use residential or rotating proxies to distribute your requests .
 * Rotate User-Agents: Mimic different browsers and devices to avoid detection .
 * Add Delays: Introduce random delays (e.g., 10-60 seconds) between requests to avoid rate-limiting.
 * Handle Dynamic Content: For complex pages, you might need tools like Playwright or Selenium to render JavaScript.
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
  const DEFAULT_DELAY = 5;

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
   * Unix timestamp of the last request.
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
    if (!db_table_exists(static::TABLE)) {
      throw new Exception('DB table ' . static::TABLE . ' not found.');
    }
    if (!empty($doi)) {
      $this->doi = $doi;
      $this->load();
      $this->getLastRequestTime();
    }
    $this->delay = variable_get(
      'tpps_google_scholar_delay', static::DEFAULT_DELAY
    );
    $this->debugMode = variable_get('tpps_google_scholar_debug_mode', FALSE);
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
    // Get data from DB.
    $result = db_select(static::TABLE, 't')
      ->fields('t')
      ->condition('doi', $this->doi)
      // Load latest record.
      ->range(0, 1)->orderBy('created')
      ->execute()->fetchAssoc();
    // Set properties.
    if ($result) {
      $this->created = $result['created'] ?? 0;
      $this->serp = $result['serp'] ?? '';
    }
    $this->isLoaded = TRUE;
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
    db_insert(static::TABLE)->fields([
      'doi' => $this->doi,
      'created' => REQUEST_TIME,
      'serp' => $this->serp,
    ])->execute();
    $this->created = REQUEST_TIME;
  }

  /**
   * Preforms request to Google Scholar web-site.
   *
   * @throws Exception
   *   When request is not allowed because of the delay.
   */
  public function request() {
    if (!$this->isRequestAllowed()) {
      throw new Exception('Request is not allowed because of the delay');
    }

    // Do the request.
    $url = $this->buildUrl();
    try {

      if ($this->debugMode) {
        $response = "test page";
      }
      else {
        $response = RestAPI::get($url);
      }
    }
    catch (Exception $e) {
      $message = $e->getMessage();
      $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3);
      watchdog('tpps', $message . '<pre>@backtrace</pre>',
        ['@backtrace' => print_r($stack, 1)],
        WATCHDOG_ERROR
      );
      throw new Exception($message, $e->getCode());
    }
    $this->serp = $response;
    $this->save();
    $this->isLoaded = TRUE;
  }

  /**
   * Gets last request time from database.
   *
   * Assume that each request is stored in database.
   * Uses public.tpps_google_scholar.created field of the latest record.
   *
   * @return int
   *   Returns latest request time as Unix timestamp.
   */
  public function getLastRequestTime() {
    $last_created = db_select(static::TABLE, 't')
      ->fields('t', ['created'])
      ->range(0, 1)->orderBy('created')
      ->execute()->fetchField();
    return $this->lastRequestTime = $last_created ?? (time() - $this->delay);
  }

  /**
   * Checks if new request is allowed.
   *
   * New request is allowed after $this->delay seconds past lastest request.
   *
   * @return bool
   *   Returns TRUE when request is allowe and FALSE otherwise.
   */
  public function isRequestAllowed() {
    if (empty($this->lastRequest)) {
      $last_created = $this->getLastRequestTime();
    }
    else {
      $last_created = $this->lastRequestTime;
    }
    return (bool) (time() - $this->delay >= $last_created);
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
    return 'https://scholar.google.com/scholar?hl=en&as_sdt=0%2C5&q='
      . urlencode(check_plain($this->doi)) . '&btnG=';
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

    // @TODO parse.
    // Store result.
  }

}
