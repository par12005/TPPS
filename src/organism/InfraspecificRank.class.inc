<?php

/**
 * @file
 * Contains 'InfraspecificRank' class.
 *
 * CV Term for 'Infraspecific Rank'.
 * chado.cvterm.name = 'speciesaggregate'.
 */

/**
 * Manages InfraspecificRank Entity.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class InfraspecificRank {

  // Name of the DB table which stores infraspecific ranks.
  const TABLE = 'chado.cvterm';

  // DB Table: chado.cv.name = 'taxonomic_rank'.
  const CV_ID = 5;

  const LIST = [
    'varietas' => [
      'cvterm_id' => 22,
      'name' => 'varietas',
      'regex' => 'variety|var.',
    ],
    'subspecies' => [
      'cvterm_id' => 29,
      'name' => 'subspecies',
      'regex' => 'subsp.|ssp.',
    ],
    'forma' => [
      'cvterm_id' => 32,
      'name' => 'forma',
      'regex' => 'form|f.|fa.',
    ],
    'subvariety' => [
      'cvterm_id' => 57,
      'name' => 'subvariety',
      'regex' => 'subvarietas|subvar.',
    ],
    'subsubvariety' => [
      'cvterm_id' => 58,
      'name' => 'subsubvariety',
      'regex' => 'sub-subvariety|subsubvar.|ssubvar.',
    ],
    'subsubform' => [
      'cvterm_id' => 62,
      'name' => 'subsubform',
      'regex' => 'sub-subform|subsubf.|ssubf.',
    ],
    'subform' => [
      'cvterm_id' => 65,
      'name' => 'subform',
      'regex' => 'subforma|subf.',
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Ranks without CVTerm Id.
    'proles' => [
      // Meghan Miles:
      // proles: I don't know how much we will see this, from looking
      // online it looks like a very old term that roughly maps
      // onto "cultivar-group" or "race/subspecies".
      'cvterm_id' => NULL,
      'name' => 'proles',
      'regex' => '',
    ],
    'nothosubsp' => [
      // Meghan Miles:
      // nothosubsp: I think we should handle this - these are hybrids
      // created by two different subspecies of the same species.
      'cvterm_id' => NULL,
      'name' => 'nothosubsp',
      'regex' => 'nothosubsp.',
    ],
    'nothovar' => [
      // Meghan Miles:
      // nothovar: a specific cultivar of a nothosubspecies (I think we
      // should handle this too, maybe - depends on difficult level,
      // although I see that this applies to a cultivar of willow).
      'cvterm_id' => NULL,
      'name' => 'nothovar',
      'regex' => 'nothovar.',
    ],
  ];

  /**
   * Gets rank name by cvterm id.
   *
   * @param int $cvterm_id
   *   Cvterm Id of the Infraspecific rank.
   *
   * @return mixed
   *   Returns (string) rank name.
   *   Returns NULL if rank name wasn't found.
   */
  public static function getName($cvterm_id) {
    foreach (static::LIST as $item) {
      if (!empty($item['cvterm_id']) && $item['cvterm_id'] == $cvterm_id) {
        return $item['name'];
      }
    }
    return NULL;
  }

  /**
   * Gets CV Term Id by rank's name, synonym or abbriviation.
   *
   * Note:
   * outdated ranks doesn't have id. See getRegex().
   *
   * @param string $rank_name
   *   Infraspecific rank name or synonym or abbriviation.
   *   Leave empty to get list of all cvterm ids.
   *
   * @return mixed
   *   If $rank_name is not empty:
   *     Returns (integer) single CV Term Id or NULL if nothing was found.
   *   If $rank_name is empty:
   *     Returns (array) list of all cv term ids of the defined ranks.
   */
  public static function getCvtermId($rank_name = NULL) {
    if (!empty($rank_name)) {
      foreach (static::LIST as $item) {
        if (
          !empty($item['cvterm_id'])
          && (
            $item['name'] == $rank_name
            || strpos($item['regex'], $rank_name) !== FALSE
          )
        ) {
          return $item['cvterm_id'];
        }
      }
      return NULL;
    }
    // Return list of the all ids.
    $list = [];
    foreach (static::LIST as $item) {
      if (!empty($item['cvterm_id'])) {
        $list[$item['name']] = $item['cvterm_id'];
      }
    }
    return $list;
  }

  /**
   * Builds regex to split organism name into parts.
   *
   * @return string
   *   Returns list of the ranks where names already has dots if necessary.
   *   Example: 'proles|subspecies ... sub-subform|subsubf.|ssubf.'.
   */
  public static function getRegex() {
    foreach (static::LIST as $data) {
      $regex[] = $data['name'];
      if (!empty($data['regex'])) {
        $regex[] = $data['regex'];
      }
    }
    $regex = implode('|', $regex);
    return $regex;
  }

}
