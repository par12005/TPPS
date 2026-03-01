<?php

/**
 * @file
 * Contains 'Gemini' class.
 *
 * Settings page: /admin/config/tpps/gemini
 * API: https://ai.google.dev/gemini-api/docs/live?example=mic-stream
 * To get/update API Key: https://aistudio.google.com/apikey
 *
 * https://ai.google.dev/api?hl=ru#text-only-prompt
 *
 * This class doesn't store response and do not use logs.
 */

/**
 * Asks Gemini AI.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class Gemini {

  /**
   * Whole Gemini API response.
   *
   * We receive JSON but $response contains PHP-array.
   *
   * @var array
   */
  protected $response;

  /**
   * Response Id.
   *
   * Example: "M5ikaY2XM57D-sAPmPSI4AY"
   *
   * @var string
   */
  protected $responseId;

  /**
   * Response.
   *
   * Note: If JSON format was requested in $generationConfig then $answer
   * will be JSON-string and must be decoded.
   *
   * $response['candidates'][0]['content']['parts'][0]['text'];
   *
   * @var string
   */
  protected $answer;

  /**
   * Gemini model name.
   *
   * Stored in variable 'tpps_gemini_model'.
   *
   * @var string
   */
  protected $model;

  /**
   * Gemini API Key.
   *
   * Stored in variable 'tpps_gemini_api_key'.
   * To get visit: https://aistudio.google.com/apikey.
   *
   * @var string
   */
  protected $apiKey;

  /**
   * Endpoint URL.
   *
   * @var string
   */
  protected $url;

  /**
   * Question's text.
   *
   * To set:
   *   $gemini->question = "Why?";
   * or
   *   $gemini->request($question = "Why?");
   * or
   *   $gemini = new Gemini($question = "Why?");
   *
   * @var string
   */
  protected $question;

  /**
   * System instructions to guide the behavior of Gemini model.
   *
   * Optional. For more details see:
   * https://ai.google.dev/gemini-api/docs/text-generation#system-instructions
   *
   * Usage example:
   * $gemini->systemInstruction = 'You're the cat.';
   *
   * @var string
   */
  protected $systemInstruction;

  /**
   * Configuration options for model generation and outputs.
   *
   * Not all parameters are configurable for every model.
   * https://ai.google.dev/api/generate-content#v1beta.GenerationConfig
   *
   * @var array
   */
  protected $generationConfig;

  /**
   * Tools and Agents extend the capabilities of Gemini models,
   * enabling them to take action in the world, access real-time information,
   * and perform complex computational tasks.
   *
   * https://ai.google.dev/gemini-api/docs/tools
   * Usage: $gemini->tools = [["google_search" => (object) []]];
   *
   * @var array
   */
  protected $tools;

  /**
   * Class constructor.
   *
   * @param string $question
   *   Question's text. Stored in $this->question.
   *   This question will be asked to Gemini AI.
   *
   * @throws Exception
   *   When Gemini model or API Key is empty.
   */
  public function __construct($question = NULL) {
    if (empty($this->model = variable_get('tpps_gemini_model'))) {
      throw new Exception("Gemini model wasn't set.");
    }
    if (empty($this->apiKey = variable_get('tpps_gemini_api_key'))) {
      throw new Exception("Gemini API Key is empty.");
    }
    $this->question = $question ?? NULL;
    $endpoint = 'generateContent';
    $this->url = 'https://generativelanguage.googleapis.com/v1beta'
      . '/models/' . $this->model . ':' . $endpoint;

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
   * Performs single request to Gemini API.
   *
   * @param mixed $question
   *   Optional. Question's text.
   *   When empty $this->question will be used.
   *
   * @return array
   *   Returns response.
   *
   * @throws Exception
   *   When $question and $this->question both are empty.
   *
   * @TODO Minor. Implement chat.
   * https://ai.google.dev/gemini-api/docs/text-generation#multi-turn-conversations
   */
  public function ask($question = NULL) {
    $question = $question ?? $this->question ?? NULL;
    if (empty($question)) {
      throw new Exception('Empty question');
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Prepare request.
    $payload['contents'] = [
      [
        'role' => 'user',
        'parts' => [['text' => $question]],
      ],
    ];
    if (!empty($this->systemInstruction)) {
      $payload['system_instruction'] = [
        'parts' => [['text' => $this->systemInstruction]],
      ];
    }
    if (!empty($this->generationConfig)) {
      $payload['generationConfig'] = $this->generationConfig;
    }
    if (!empty($this->tools)) {
      $payload['tools'] = $this->tools;
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Do the request.
    $header = [
      'x-goog-api-key: ' . $this->apiKey,
      'Content-Type: application/json',
    ];

    try {
      $json_response = RestAPI::post($this->url, $payload, $header);
      $response = json_decode($json_response, TRUE);
    }
    catch (Exception $e) {
      // @TODO Minor. Code 429 (Too many requiests).
      $message = $e->getMessage();
      $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3);
      watchdog('tpps', $message . '<pre>@backtrace</pre>',
        ['@backtrace' => print_r($stack, 1)],
        WATCHDOG_ERROR
      );
      // throw new Exception($message, $e->getCode());
      drupal_set_message($e->getCode() . ' ' . $message);
    }

    $this->response = $response;
    $this->responseId = $response['responseId'];
    $this->answer = $response['candidates'][0]['content']['parts'][0]['text'];
    return $this->answer;
  }

}
