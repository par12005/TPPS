<?php

/**
 * @file
 * Contains static 'PublicationDoi' class.
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
class PublicationDoi extends DOI {

  // Name of the DB table where stored DOI publication information.
  const TABLE = 'tpps_publication_doi';

  // This key used in a list of DOI to separate DOI which was already parsed and has publication data.
  // Used in the dropdown menu and suggestions.
  const PD_KEY = 'Has publication data';
  // Key used to separate DOIs which has no publication data yes.
  // Used in the dropdown menu and suggestions.
  const NOPD_KEY = "Doesn't have publication data";

  /**
   * Checks if the record for given DOI exists in DB table.
   *
   * @param string $doi
   *   DOI.
   * @param string $source
   *   Required. Source of the publication data.
   *   Possible values: see DoiSerp::KEY.
   *
   * @return bool
   *   Returns TRUE if record exists and FALSE otherwise.
   */
  public static function isExists($doi, $source) {
    $query = db_select(self::TABLE, 't')
      ->fields('t', ['doi'])
      ->condition('doi', $doi)
      ->range(0, 1)
      ->condition('source', $source);
    return (bool) $query->execute()->fetchField();
  }

  /**
   * Checks if the record for given DOI is missing in DB table.
   *
   * @param string $doi
   *   DOI.
   * @param string $source
   *   Required. Source of the publication data.
   *   Possible values: see DoiSerp::KEY.
   *
   * @return bool
   *   Returns TRUE if record is missing and FALSE otherwise.
   */
  public static function isMissing($doi, $source) {
    return !PublicationDoi::isExists($doi, $source);
  }

  /**
   * Checks if the DB record for given DOI is empty.
   *
   * @param string $doi
   *   DOI.
   * @param string $source
   *   Optional. Source of the publication data.
   *   Possible values: see DoiSerp::KEY and DoiApi::KEY.
   *   When empty then 'combined' version of the DOI Publication data
   *   will be checked.
   *
   * @return bool
   *   When $source is not empty checks if 'title' field is not empty because
   *   other fields could be empty (for example 'abstract' or 'journal').
   *
   *   When $source is empty (combined version) then returns TRUE if any of
   *   the fields is empty and FALSE when all fields are not empty.
   */
  public static function isEmpty($doi, $source = NULL) {
    $data = static::load($doi, $source);
    if (empty($source)) {
      // Check 'combined' version.
      // Return FALSE if all values of $combined are not empty.
      return (empty($data) || count($data) !== count(array_filter($data)));
    }
    else {
      // Note: We check only title because some data sources has no 'abstract'
      // or no 'journal' fields.
      return empty($data['title']);
    }
  }

  /**
   * Saves publication data into DB table.
   *
   * @param array $data
   *   See self::TABLE columns.
   *
   * @return bool
   *   Returns TRUE in case of success and FALSE otherwise.
   *
   * @throws Exception
   *   When record for given $data['doi'] already exists.
   */
  public static function save(array $data = []) {
    if (empty($data) || empty($data['doi']) || empty($data['source'])) {
      return FALSE;
    }
    $is_record_exists = PublicationDoi::isExists($data['doi'], $data['source']);
    if ($is_record_exists) {
      $message = 'Record already exists. DOI: @doi, Source: @source';
      $params = [
        '@doi' => $data['doi'],
        '@source' => $data['source'],
      ];
      throw new Exception(t($message, $params));
    }
    else {
      if (!empty($data['authors']) && is_array($data['authors'])) {
        $data['authors'] = implode(', ', $data['authors']);
      }
      db_insert(self::TABLE)->fields($data)->execute();
      return TRUE;
    }
  }

  /**
   * Removes record from DB.
   *
   * @param string $doi
   *   DOI.
   * @param string $source
   *   Data provider class name.
   */
  public static function remove($doi, $source) {
    db_delete(self::TABLE)
      ->condition('doi', $doi)
      ->condition('source', $source)
      ->execute();
  }

  /**
   * Replaces existing record in DB with new data.
   *
   * @param array $data
   *   DOI publication data.
   *
   * @return bool
   *   Returns TRUE if record was replaced and FALSE otherwise.
   */
  public static function replace(array $data) {
    if (!empty($data['doi']) && !empty($data['source'])) {
      // @todo Minor. Check if $fields is better then existing data in DB.
      PublicationDoi::remove($data['doi'], $data['source']);
      PublicationDoi::save($data);
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Loads publication data from DB table.
   *
   * @param string $doi
   *   Required. DOI (Digital Object Identifier).
   * @param string $source
   *   Optional. Source of the publication data.
   *   Possible values:
   *   'all', empty and constants DoiSerp::KEY and DoiApi::KEY.
   * @param bool $reset
   *   Default is FALSE. When TRUE static cache will be reset.
   *
   * @return array
   *   Returns record from DB table for given DOI if exists.
   *   Returns empty array if record wasn't found.
   *
   * @throws Exception
   *   When $source is invalid.
   */
  public static function load($doi, $source = NULL, $reset = FALSE) {
    $output = [];
    $use_check_plain = FALSE;

    $static_cache = &drupal_static(__METHOD__);
    $cache_key = $doi . $source;

    if ($reset || !isset($static_cache[$cache_key])) {
      if (empty($source)) {
        // Combined.
        // Rule: longer string is better.
        $data = db_select(static::TABLE, 't')
          ->fields('t', [])
          ->condition('doi', $doi)
          ->execute()
          ->fetchAllAssoc('source', PDO::FETCH_ASSOC);
        $combined = [];
        foreach ($data as $source => $item) {
          foreach ($item as $key => $value) {
            if ($key == 'source') {
              continue;
            }
            if ($use_check_plain) {
              $value = check_plain($value);
            }

            if (empty($combined[$key])) {
              $combined[$key] = $value;
            }
            else {
              // Get number of letters in new string.
              $value_letters = preg_replace('/[^\p{L}]/u', '', $value);
              $existing_letters = preg_replace('/[^\p{L}]/u', '', $combined[$key]);
              // Assume that longer string is better (more data).
              if (strlen($existing_letters) < strlen($value_letters)) {
                $combined[$key] = $value;
              }
            }
          }
        }
        $output = $combined;
      }
      elseif ($source == 'all') {
        // All sources.
        $output = db_select(static::TABLE, 't')
          ->fields('t', [])
          ->condition('doi', $doi)
          ->execute()
          ->fetchAllAssoc('source', PDO::FETCH_ASSOC);
        if ($use_check_plain) {
          foreach ($output as $source => $source_data) {
            $output[$source] = array_map('check_plain', $source_data);
          }
        }
      }
      else {
        // Single source.
        $output = db_select(static::TABLE, 't')
          ->fields('t', [])
          ->condition('doi', $doi)
          ->condition('source', $source)
          ->range(0, 1)
          ->execute()
          ->fetchAssoc();
        if ($use_check_plain) {
          $output = array_map('check_plain', $output);
        }
      }
      $static_cache[$cache_key] = $output;
    }
    return $static_cache[$cache_key];
  }

  /**
   * Gets DB table schema.
   *
   * Used in hook_install().
   * Table name: self::TABLE.
   *
   * Usage example:
   *   $field_list = array_keys(self::getSchema()['fields']);
   *   unset($field_list['source']);
   *
   * @return array
   *   Returns db table schema.
   */
  public static function getSchema() {
    return [
      'description' => 'Stores DOI publication information.',
      'fields' => [
        'doi' => [
          'description' => 'DOI (Digital Object Identifier).',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'source' => [
          'description' => 'Source of the data. Class name of the data provider.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => TRUE,
        ],
        'journal' => [
          'description' => 'Publication Journal.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => FALSE,
        ],
        'date' => [
          'description' => 'Publication Date.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => FALSE,
        ],
        'title' => [
          'description' => 'Publication Title.',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => FALSE,
        ],
        'authors' => [
          'description' => 'Authors (primary and secondary).',
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => FALSE,
        ],
        'abstract' => [
          'description' => 'Publication Abstract.',
          // varchar/normal: mysql: 64KB, postgresql: 1GB.
          'type' => 'varchar',
          'size' => 'normal',
          'not null' => FALSE,
        ],
      ],
      'unique keys' => [
        'doi_source' => ['doi', 'source'],
      ],
    ];
  }

  /**
   * Gets list of the DOI used at live site.
   *
   * For debugging/testing.
   *
   * @return array
   *   Returns list of the real DOI.
   */
  public static function getList() {
    return [
      '10.1007/s13595-021-01027-6',
      '10.1186/s13104-021-05903-2',
      '10.5061/dryad.1237',
      '10.1111/mec.14341',
      '10.5061/dryad.n818s',
      '10.5061/dryad.860jq72n',
      '10.5061/dryad.5cm32',
      '10.5061/dryad.jp13f',
      '10.1007/s10592-019-01195-z',
      '10.1002/ajb2.1715',
      '10.5061/dryad.1j86179',
      '10.5061/dryad.43rv3',
      '10.5061/dryad.vk67d',
      '10.5061/dryad.s11b6',
      '10.5061/dryad.k9b4n',
      '10.5061/dryad.r759h',
      '10.5061/dryad.rn8pk0pbt',
      '10.5061/dryad.2r6m4',
      '10.1007/s11295-024-01657-7',
      '10.1111/jbi.13874',
      '10.1111/j.1365-294X.2010.04559.x',
      '10.5061/dryad.mk968sm',
      '10.5061/dryad.59zw3r243',
      '10.1038/hdy.2015.57',
      '10.5061/dryad.6s82f20',
      '10.5061/dryad.244b8',
      '10.3732/apps.1300025',
      '10.5061/dryad.1p145',
      '10.1007/s11295-015-0890-x',
      '10.5061/dryad.sj3tx963w',
      '10.5061/dryad.6r725',
      '10.5061/dryad.pb128',
      '10.5061/dryad.9kb02',
      '10.5061/dryad.1vp70',
      '10.5061/dryad.s0vf6',
      '10.5061/dryad.r7sqv9s7k',
      '10.5061/dryad.bq002',
      '10.5061/dryad.h55q8',
      '10.5061/dryad.269126c',
      '10.1139/cjb-2022-0088',
      '10.1111/j.1365-294X.2012.05555.x',
      '10.5061/dryad.fv322',
      '10.1038/hdy.2013.132',
      '10.5061/dryad.1nq6m',
      '10.1111/mec.13269',
      '10.5061/dryad.10j72',
      '10.5061/dryad.2fqz612kc',
      '10.1002/ajb2.1725',
      '10.1038/s41598-023-37974-5',
      '10.5061/dryad.b56tm0t',
      '10.5061/dryad.jh21r',
      '10.5061/dryad.g79cnp5n2',
      '10.5061/dryad.dn53j',
      '10.7717/peerj.13565',
      '10.1007/s11295-016-1059-y',
      '10.5061/dryad.s9n91',
      '10.5061/dryad.w6m905qkc',
      '10.5061/dryad.np8708k2',
      '10.1111/j.1365-294X.2012.05591.x',
      '10.5061/dryad.rr4xgxd6s',
      '10.5061/dryad.67529',
      '10.5061/dryad.0r20b',
      '10.5061/dryad.fv348',
      '10.5061/dryad.vt45p',
      '10.1111/mec.13348',
      '10.1007/s10531-018-1628-5',
      '10.1002/ece3.3906',
      '10.1186/s12862-021-01805-w',
      '10.1002/ece3.669',
      '10.1016/j.gene.2021.145791',
      '10.5061/dryad.2dg7c',
      '10.5061/dryad.435s1',
      '10.1007/s00442-024-05607-x',
      '10.1186/s12862-016-0810-1',
      '10.5061/dryad.b78fb',
      '10.1111/jbi.14586',
      '10.5061/dryad.b2q88',
      '10.5061/dryad.08gc2',
      '10.5061/dryad.t8q1g',
      '10.1111/j.1744-7429.2007.00371.x',
      '10.1038/s41437-018-0132-8',
      '10.1186/s12870-025-06324-0',
      '10.1093/jhered/esw038',
      '10.1371/journal.pone.0241025',
      '10.1111/btp.12839',
      '10.1007/s00122-010-1350-5',
      '10.5061/dryad.s205vd8',
      '10.1111/1755-0998.13469',
      '10.5061/999',
      '10.1098/rspb.2019.0486',
      '10.1002/ece3.10376',
      '10.1111/nph.16862',
      '10.5061/dryad.70q32',
      '10.5061/dryad.9ghx3ffd4',
      '10.1002/ece3.10145',
      '10.1038/hdy.2016.40',
      '10.3389/fpls.2016.01244',
      '10.1371/journal.pone.0016056',
      '10.5061/dryad.dp7dv00',
      '10.5061/dryad.1p2s5',
      '10.5061/dryad.p12cn93',
      '10.5061/dryad.stqjq2c0q',
      '10.1007/s11295-023-01619-5',
      '10.5061/dryad.13kr7',
      '10.1093/jhered/esq118',
      '10.5061/dryad.76nv3',
      '10.5061/dryad.37r9p',
      '10.5061/dryad.50hf0',
      '10.1111/jbi.14113',
      '10.1002/ece3.6189',
      '10.1080/11263504.2017.1415993',
      '10.5061/dryad.h3d26',
      '10.5061/dryad.mb4mv6pm',
      '10.5061/dryad.45t51',
      '10.5061/dryad.n193d',
      '10.5061/dryad.26f7v',
      '10.5061/dryad.b2pg468',
      '10.1371/journal.pone.0186757',
      '10.5061/dryad.3dv00',
      '10.5061/dryad.pk8997k0',
      '10.1111/jbi.13726',
      '10.5061/dryad.7kk62',
      '10.5061/dryad.7tr51',
      '10.5061/dryad.8q51152',
      '10.5061/dryad.9nm58',
      '10.5061/dryad.39fs7',
      '10.7717/peerj.17554',
      '10.3389/fpls.2022.990927',
      '10.5061/dryad.kv573c7',
      '10.1002/ece3.4575',
      '10.5061/dryad.6pg15js',
      '10.5061/dryad.p8cz8w9kp',
      '10.5061/dryad.55c2f',
      '10.5061/dryad.c014p',
      '10.3390/plants10081572',
      '10.5061/dryad.4mg2r',
      '10.5061/dryad.6p1c4',
      '10.5061/dryad.36634',
      '10.5061/dryad.g0141g7',
      '10.5061/dryad.cp91mj7',
      '10.1534/genetics.110.115543',
      '10.5061/dryad.c959s',
      '10.5061/dryad.rv15dv49q',
      '10.1038/s41598-022-17635-9',
      '10.5061/dryad.15pg4g4',
      '10.1111/jbi.14002',
      '10.5061/dryad.2vc0n',
      '10.5061/dryad.1mt1h',
      '10.5061/dryad.dj17c',
      '10.6078/D10Q3N',
      '10.5061/dryad.ghx3ffbng',
      '10.5061/dryad.5f41fq18',
      '10.5061/dryad.hf49c',
      '10.5061/dryad.0p264',
      '10.5061/dryad.215sn',
      '10.5061/dryad.31rq4',
      '10.1007/s11258-011-0003-3',
      '10.1093/aobpla/plaa069',
      '10.5061/dryad.r6fr1',
      '10.5061/dryad.08507r35',
      '10.5061/dryad.g8r7t',
      '10.5061/dryad.nb320',
      '10.5061/dryad.cc03n',
      '10.1002/ece3.7077',
      '10.5061/dryad.t84p3',
      '10.5061/dryad.2280gb5n0',
      '10.1007/s00035-020-00234-7',
      '10.5061/dryad.dbrv15dxb',
      '10.1186/s12870-020-02374-8',
      '10.46265/genresj.ALFV3636',
      '10.3389/fpls.2018.00532',
      '10.1371/journal.pone.0248954',
      '10.3390/f14040857',
      '10.5061/dryad.r1f08nt',
      '10.5061/dryad.gg3g3s5',
      '10.5061/dryad.3ck30',
      '10.1111/nph.14266',
      '10.5061/dryad.3rn6323d',
      '10.5061/dryad.63xsj3tzx',
      '10.1038/hdy.2016.64',
      '10.1111/mec.15277',
      '10.1002/ece3.7613',
      '10.5061/dryad.ttdz08m1k',
      '10.5061/dryad.d5672',
      '10.5061/dryad.54527',
      '10.1007/s10528-019-09904-6',
      '10.5061/dryad.qnk98sff6',
      '10.1186/s12870-016-0947-5',
      '10.5061/dryad.gp2tc',
      '10.5061/dryad.gg0jm',
      '10.5061/dryad.z08kprr9v',
      '10.5061/dryad.rg82f',
      '10.5061/dryad.6f3q5',
      '10.1186/1471-2164-12-145',
      '10.5061/dryad.h9022',
      '10.1186/s12862-015-0331-3',
      '10.1371/journal.pone.0163683',
      '10.5061/dryad.mcvdnck29',
      '10.5061/dryad.78j501j',
      '10.5061/dryad.p3v3s',
      '10.1002/ece3.8169',
      '10.5061/dryad.5sb1219',
      '10.1007/s11295-023-01617-7',
      '10.3732/ajb.1600336',
      '10.5061/dryad.t429r75',
      '10.5061/dryad.c3h3v',
      '10.5061/dryad.h8fp2',
      '10.1371/journal.pone.0241452',
      '10.5061/dryad.873b2j2',
      '10.1007/s12686-019-01110-1',
      '10.5061/dryad.82kv2',
      '10.1007/s11295-014-0721-5',
      '10.25338/B8864J',
      '10.1111/1365-2435.13229',
      '10.1111/nph.20026',
      '10.3389/fgene.2019.01384',
      '10.1111/mec.14241',
      '10.5061/dryad.tm8d0',
      '10.7717/peerj.10922',
      '10.1111/jbi.14054',
      '10.5061/dryad.4tm3v',
      '10.1038/s41437-018-0151-5',
      '10.5061/dryad.fb436',
      '10.5061/dryad.6644t',
      '10.1111/eva.12474',
      '10.1016/j.foreco.2023.120801',
      '10.1111/mec.12746',
      '10.5061/dryad.37354',
      '10.1111/jbi.13650',
      '10.1038/hdy.2012.40',
      '10.1038/s41437-017-0012-7',
      '10.1007/s00606-021-01744-5',
      '10.1111/mec.14820',
      '10.5061/dryad.np29h',
      '10.5061/dryad.53pn8',
      '10.5061/dryad.2280gb5pd',
      '10.5061/dryad.qfttdz0jw',
      '10.5061/dryad.g94676pg',
      '10.5061/dryad.1339',
      '10.1007/s10592-016-0835-4',
      '10.1007/s11032-019-0993-4',
      '10.1038/s41437-020-00387-5',
      '10.1111/mec.13889',
      '10.5061/dryad.hg77n',
      '10.1007/s11295-014-0779-0',
      '10.5061/dryad.pt3b974',
      '10.5061/dryad.pm251177',
      '10.5061/dryad.p49q0',
      '10.1111/mec.13804',
      '10.5061/dryad.1cm80',
      '10.1111/jbi.14612',
      '10.5061/dryad.78ng1',
      '10.1371/journal.pone.0184456',
      '10.1111/1365-2745.13670',
      '10.5061/dryad.b5s47',
      '10.5061/dryad.6q0q4',
      '10.1038/ng.3612',
      '10.1007/s00122-017-3045-7',
      '10.3390/f12091164',
      '10.3389/fcosc.2023.1206832',
      '10.5061/dryad.g79cnp5mt',
      '10.5061/dryad.9067',
      '10.1002/ece3.6200',
      '10.1007/s11295-017-1189-x',
      '10.5061/dryad.bm46p',
      '10.1007/s11295-016-1062-3',
      '10.1007/s11295-015-0867-9',
      '10.5061/dryad.s3d45',
      '10.1002/ece3.6271',
      '10.5061/dryad.5r3b1',
      '10.1007/s11295-019-1391-0',
      '10.5061/dryad.42075m1',
      '10.1007/s11295-013-0669-x',
      '10.3389/fpls.2020.00830',
      '10.5061/dryad.mkkwh70wf',
      '10.5061/dryad.1280',
      '10.5061/dryad.tf556',
      '10.5061/dryad.v3t16d8',
      '10.5061/dryad.d9b02tp',
      '10.3390/f11080862',
      '10.5061/dryad.hqbzkh1d7',
      '10.1038/hdy.2012.103',
      '10.1007/s11295-017-1147-7',
      '10.1534/genetics.114.173252',
      '10.1007/s11295-017-1174-4',
      '10.5061/dryad.6f64b',
      '10.5061/dryad.d6396sd',
      '10.5061/dryad.j0zpc86f4',
      '10.1186/s12870-023-04428-z',
      '10.1002/tax.12096',
      '10.21267/IN.2016.6.2294',
      '10.5061/dryad.c5c1q',
      '10.1002/ajb2.1504',
      '10.3390/plants12132557',
      '10.5061/dryad.mg5rs',
      '10.1111/jse.12818',
      '10.1371/journal.pone.0286480',
      '10.1007/s00606-020-01726-z',
      '10.5061/dryad.1767',
      '10.5061/dryad.2593t',
      '10.1111/btp.12531',
      '10.1111/j.1365-294X.2011.05394.x',
      '10.5061/dryad.855pg',
      '10.5061/dryad.nv066',
      '10.1111/mec.13942',
      '10.5061/dryad.n7dv6',
      '10.1007/s11295-013-0612-1',
      '10.5061/dryad.851d6k5',
      '10.5061/dryad.77j02',
      '10.5061/dryad.p502g',
      '10.1371/journal.pone.0015529',
      '10.1007/s10592-020-01281-7',
      '10.1111/mec.12129',
      '10.1111/mec.15264',
      '10.5061/dryad.xksn02vcs',
      '10.5061/dryad.d4t7rc3',
      '10.5061/dryad.8075',
      '10.5061/dryad.467g5',
      '10.1111/jbi.12843',
      '10.5061/dryad.8dq043h',
      '10.5061/dryad.08kprr528',
      '10.1111/eva.12792',
      '10.5061/dryad.70rxwdbtf',
      '10.1016/j.jasrep.2020.102640',
      '10.1038/s41598-020-71918-7',
      '10.1139/cjb-2022-0107',
      '10.1111/mec.12554',
      '10.1111/j.1469-8137.2007.02213.x',
      '10.5061/dryad.k3306',
      '10.1111/1755-0998.12056',
      '10.5061/dryad.bm8458m',
      '10.5061/dryad.c58539p',
      '10.5061/dryad.92j7j53',
      '10.5061/dryad.mk58d',
      '10.1186/s12870-024-04942-8',
      '10.5061/dryad.ck44d',
      '10.5061/dryad.267kc',
      '10.1038/hdy.2016.130',
      '10.1002/ece3.4216',
      '10.1002/ece3.6768',
      '10.5061/dryad.r17ss',
      '10.1371/journal.pone.0159545',
      '10.1111/1365-2745.13639',
      '10.1371/journal.pone.0106874',
      '10.5061/dryad.v025v',
      '10.1016/j.ympev.2015.01.006',
      '10.1093/forestry/cpy020',
      '10.5061/dryad.8ff54',
      '10.1111/mec.12567',
      '10.5061/dryad.tm33d',
      '10.1111/j.1744-7429.2011.00836.x',
      '10.1111/nph.14380',
      '10.1111/nph.12815',
      '10.1111/mec.12336',
      '10.5061/dryad.27jp751',
      '10.1371/journal.pone.0167104',
      '10.1186/s43141-021-00168-5',
      '10.5061/dryad.j3s5f',
      '10.1038/hdy.2015.77',
      '10.1111/1365-2664.14667',
      '10.5061/dryad.n02v6wwt2',
      '10.5061/dryad.ms580',
      '10.1038/hdy.2014.16',
      '10.1111/nph.12601',
      '10.3390/biom14080944',
      '10.1111/eva.13535',
      '10.5061/dryad.588d7',
      '10.1111/mec.12231',
      '10.1111/aec.12223',
      '10.1371/journal.pone.0231263',
      '10.1371/journal.pone.0151730',
      '10.5061/dryad.hq8pd',
      '10.5061/dryad.sf7m0cg98',
      '10.1038/sdata.2015.6',
      '10.1093/jhered/esaa011',
      '10.5061/dryad.6bf38',
      '10.1007/s10342-024-01687-9',
      '10.5061/dryad.mq8fn',
      '10.5061/dryad.56j8vq8',
      '10.1007/s11295-012-0569-5',
      '10.5061/dryad.84vc24v',
      '10.1111/boj.12353',
      '10.1002/ece3.6700',
      '10.1186/s12862-018-1194-1',
      '10.1093/jhered/esw009',
      '10.1093/aob/mcab140',
      '10.5061/dryad.kf1td',
      '10.5061/dryad.gv5qp70',
      '10.1371/journal.pone.0196652',
      '10.5061/dryad.qj3hn',
      '10.3389/fgene.2021.733576',
      '10.1038/s41437-024-00684-3',
      '10.1002/ece3.7944',
      '10.1371/journal.pone.0127516',
      '10.5061/dryad.c1q38',
      '10.5061/dryad.473n5',
      '10.5061/dryad.bk5hk',
      '10.5061/dryad.gs352',
      '10.1111/nph.13711',
      '10.5061/dryad.12kh6',
      '10.1111/njb.01747',
      '10.1111/mec.15833',
      '10.1073/pnas.0902213106',
      '10.1111/jbi.14215',
      '10.1002/ece3.5833',
      '10.1038/s41598-020-74123-8',
      '10.1111/mec.16809',
      '10.1111/mec.12639',
      '10.5061/dryad.8hh6g56',
      '10.5061/dryad.7qc3s',
      '10.1111/mec.15046',
      '10.1371/journal.pone.0160745',
      '10.1002/ajb2.16019',
      '10.5061/dryad.4b18b',
      '10.7717/peerj.16735',
      '10.1371/journal.pone.0116411',
      '10.1002/ece3.6537',
      '10.1111/mec.15147',
      '10.1002/ajb2.1396',
      '10.3732/ajb.1500278',
      '10.1007/s11295-019-1388-8',
      '10.5061/dryad.3274n',
      '10.1002/ece3.10670',
      '10.1093/jhered/esw078',
      '10.1186/s12870-024-05732-y',
      '10.1111/jse.12594',
      '10.5061/dryad.3j300',
      '10.1038/s41437-018-0091-0',
      '10.1093/aob/mcz129',
      '10.1111/eva.12472',
      '10.1111/mec.12317',
      '10.1038/hdy.2016.128',
      '10.1371/journal.pone.0247586',
      '10.5061/dryad.f1vhhmgtk',
      '10.1111/mec.14139',
      '10.1111/mec.14181',
      '10.1038/hdy.2016.65',
      '10.1111/mec.13207',
      '10.1111/mec.12406',
      '10.1007/s11295-015-0965-8',
      '10.5061/dryad.3gq01',
      '10.1002/ajb2.1244',
      '10.1002/ece3.7529',
      '10.1111/j.1365-294X.2012.05486.x',
      '10.5061/dryad.pq6hg',
      '10.1111/ddi.13588',
      '10.1111/j.1365-294X.2011.05203.x',
      '10.3389/fevo.2019.00170',
      '10.1093/jhered/esx096',
      '10.1111/j.1755-0998.2011.03094.x',
      '10.1111/mec.15365',
      '10.1038/s41598-019-46275-9',
      '10.1002/ece3.5113',
      '10.1038/hdy.2013.31',
      '10.1111/eva.12064',
      '10.1002/eap.2254',
      '10.5061/dryad.kq0n5',
      '10.1093/botlinnean/boab042',
      '10.1093/aob/mcae216',
      '10.1111/mec.15383',
      '10.1371/journal.pone.0192970',
      '10.1111/mec.12207',
      '10.3732/ajb.1300223',
      '10.1016/j.pld.2024.03.009',
      '10.1007/s00122-008-0955-4',
      '10.1093/jhered/est065',
      '10.1093/biolinnean/blx168',
      '10.1111/j.1365-294X.2011.05229.x',
      '10.5061/dryad.3801d',
      '10.1002/ece3.7473',
      '10.1038/s41437-017-0035-0',
      '10.5061/dryad.bq4p7',
      '10.1002/ece3.6829',
      '10.5061/dryad.6pq0n',
      '10.5061/dryad.gs27k',
      '10.1111/mec.12442',
      '10.1038/hdy.2015.100',
      '10.1111/mec.12969',
    ];
  }

  /**
   * Builds 2 level list of DOIs.
   *
   * @return array
   *   Returns 2 level array.
   */
  public static function buildOptions() {
    $yes = t('Has publication data');
    $no = t("Doesn't have publication data");
    $output = [
      static::PD_KEY => [],
      static::NOPD_KEY => [],
    ];

    // @todo Cache data.
    $all_doi_list = PublicationDoi::getList();
    $parsed_doi_list = db_select(PublicationDoi::TABLE, 't')
      ->fields('t', ['doi'])
      ->groupBy('doi')
      ->execute()
      ->fetchCol('doi');

    foreach ($all_doi_list as $value) {
      if (in_array($value, $parsed_doi_list)) {
        // Processed.
        $output[static::PD_KEY][$value] = $value;
      }
      else {
        // Not yet processed.
        $output[static::NOPD_KEY][$value] = $value;
      }
    }
    return $output;
  }

  /**
   * Gets combined publication data from all APIs and SERP providers.
   *
   * If data from API is missing it will be requested and stored in DB.
   * If data from SERP provider is missing but RestApi has saved data it will
   * be reused to parse SERP.
   *
   * @param string $doi
   *   Required. DOI (Digital Object Identifier).
   */
  public static function getCombined($doi, $source = NULL) {
    // When TRUE debug watchdog messages will be created.
    $debug_mode = FALSE;
    $data = PublicationDoi::load($doi);
    if (!empty($data) && PublicationDoi::isComplete($data)) {
      // We already have all the data.
      return $data;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Data is incomplete. Let's get data from other sources.
    $api_list = static::getApiList();
    $class_list = array_values($api_list);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $allowed_api_list = array_filter(array_values(
      variable_get('tpps_doi_lookup_allowed_api_list')
    ));
    if (!empty($allowed_api_list)) {
      if ($debug_mode ?? FALSE) {
        $message = 'DOI: @doi. API to be checked: @class_list';
        $params = ['@doi' => $doi, '@class_list' => implode(', ', $class_list)];
        watchdog(__METHOD__, $message, $params);
      }
      // @todo Minor. Run requests in parallel.
      $method_name = 'insert';
      foreach ($class_list as $class_name) {
        if (
          in_array($class_name, $allowed_api_list)
          && class_exists($class_name)
          && method_exists($class_name, $method_name)
        ) {
          $class_name::$method_name($doi);
        }
      }
      $data = PublicationDoi::load($doi, NULL, TRUE);
    }

    return $data;
  }

  /**
   * Gets list of the existing APIs which provide DOI publication data.
   *
   * @return array
   *   Returns associative array where keys are API machine names
   *   (class's KEY constant) and value is human readable title.
   *
   * @TODO Use only class names.
   */
  public static function getApiList() {
    return [
      // $source => $class_name,
      'semantic_scholar' => 'SemanticScholar',
      'crossref' => 'CrossRef',
      'dryad' => 'Dryad',
      'pubmed_central' => 'PubMedCentral',
    ];
  }

  /**
   * Gets list of the supported SERP providers.
   *
   * @return array
   *   Returns associative array where keys are API machine names
   *   (class's KEY constant) and value is human readable title.
   */
  public static function getSerpList() {
    return [
      // $key => $class_name,
      'pubmed' => 'PubMed',
      'google_scholar' => 'GoogleScholar',
      // 'datadryad' => 'DataDryad',
    ];
  }

  /**
   * Builds list of APIs which wasn't yet requested.
   *
   * DOI could have no publication data from some data providers because they
   * could be disabled on admin settings page.
   *
   * @param string $doi
   *   DOI.
   *
   * @return array
   *   Returns list of the class names of the API services.
   *
   * @TODO Update requestAPI() to request only API which data is missing.
   * @TODO Ability to force: ignore allowed API list.
   */
  public static function getMissingApiList($doi) {
    $data = PublicationDoi::load($doi, 'all');
    //dpm($data);

    return;
    // Not yet implemented.

    $all_api_list = static::getApiList();
    $allowed_api = variable_get('tpps_doi_lookup_allowed_api_list');
    foreach ($data as $class_name => $value) {
      if (!in_array($class_name, $allowed_api_list)) {
        continue;
      }

    }

    return $missing_list;
  }

  /**
   * Checks if given $data has no empty values.
   *
   * @param array $data
   *   Array to be checked.
   *
   * @return bool
   *   Returns TRUE if given array has no empty values and FALSE otherwise.
   */
  public static function isComplete(array $data) {
    return (!empty($data) && is_array($data) && TppsArray::isComplete($data));
  }

}
