<?php

/**
 * @file
 * Contains 'ZenodoBucket' class.
 */

/**
 * Zenodo API for Deposition Files.
 *
 * See: https://developers.zenodo.org/#deposition-files.
 *
 * Inherits CRUD actions for files from parent 'Zenodo' class:
 *   create()
 *   delete(array $data)
 *   list(array $data)
 *   retrieve(array $data)
 *   update(array $data)
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class ZenodoBucket extends Zenodo {

  /**
   * Creates a blank Zenodo DOI Deposit/Files.
   *
   * Scope (permissions): deposit:write.
   *
   * @param array $data
   *   Data for request.
   *   When $data['accession'] is not set then empty Zenodo Deposition will
   *   be created. When it's not empty and has TPPS Submission accession
   *   (e.g., 'TGDRxxxx) then $data will be populated with study's data.
   *
   * @return array
   *   Returns Zenodo Deposition data or empty array.
   */
  public static function create(array $data = []) {
    $data['payload'] = $data['payload'] ?? [];
    $data['header'] = $data['header'] ?? [];

    // Call all implementations of hooks:
    // - hook_zenodo_bucket_create_alter(),
    drupal_alter(static::getHookName(__FUNCTION__), $data);

    $data['url'] = static::buildUrl($data);
    $data['response'] = RestAPI::put($data['url'], $data['payload'], $data['header']);

    // hook_zenodo_bucket_created(),
    module_invoke_all(static::getHookName('created'), $data);
    return $data['response'];
  }

         //$cFile = new CURLFile(
         //  tpps_realpath($file->uri),
         // //$file->filemime,
         // 'application/octet-stream',
         // $file->filename
         //);
          //$filepath = tpps_realpath($file->uri);
          //$fp = '@' . $filepath;
          //$size = filesize($filepath);

          ////$file = curl_file_create($this->file, 'application/octet-stream', 'EcommProdotti');


          //curl_setopt($curl, CURLOPT_PUT, true);
          //curl_setopt($curl, CURLOPT_CUSTOMREQUEST, 'POST');
          //curl_setopt($curl, CURLOPT_INFILE, $fp);
          //curl_setopt($curl, CURLOPT_INFILESIZE, $size);
}
