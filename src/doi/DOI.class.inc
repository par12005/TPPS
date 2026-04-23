<?php

/**
 * @file
 * Contains static 'DOI' class.
 *
 * DOI Format.
 *
 * The first part of a DOI (before the /) is called the DOI prefix (or prefix).
 * For the DOI 10.1002/ajb2.1504:
 *   Prefix: 10.1002 (prefix for Wiley)
 *     "10" is the directory indicator (always part of any DOI prefix).
 *     "1002" is the registrant code (unique to the organization that pays
 *            for and assigns DOIs).
 *   Suffix: ajb2.1504
 */

/**
 * Manages DOI for TPPS.
 *
 * @package tpps
 * @author Vlad Savitjsky <vlad.savitsky@gmail.com>
 */
class DOI {

  // Single DOI Regexp which could be used in preg_match() as $pattern.
  const REGEX = '/^10\.\d{4,9}[\-._;()\/:A-Za-z0-9]+$/';
  // Multiple DOIs Regex to find all matches.
  const SEARCH_REGEX = '/10\.\d{4,9}[\-._;()\/:A-Za-z0-9]+/';

  /**
   * Prints DOI as hyperlink.
   *
   * @param string $doi
   *   Optional. DOI.
   *
   * @return string
   *   Returns rendered HTML tag <A> where anchor is a DOI and href is a link to
   *   https://doi.org/ site which will redirect to the publication.
   *   If $doi is empty returns empty string.
   */
  public static function print($doi = NULL) {
    if (!empty($doi)) {
      $output = theme('doi_link', ['doi' => $doi]);
    }
    return $output ?? '';
  }

  /**
   * Gets first DOI in given string.
   *
   * @param string $string
   *   String where DOI will be searched.
   *
   * @return string
   *   Returns first DOI found in string.
   */
  public static function searchFirst($string) {
    preg_match(static::SEARCH_REGEX, $string, $matches);
    return $matches[0] ?? NULL;
  }

  /**
   * Searches DOI in given string.
   *
   * @param string $string
   *   String where DOI will be searched.
   *
   * @return string
   *   Returns first found
   */
  public static function search($string) {
    preg_match_all(static::SEARCH_REGEX, $string, $matches);
    return $matches[0] ?? NULL;
  }

  /**
   * Checks if given string is a valid DOI.
   *
   * WARNING:
   * Use trim() before call this method.
   * Leading/trailing spaces are not allowed.
   *
   * @param string $doi
   *   String which contains DOI.
   *
   * @return bool
   *   Returns TRUE if given string is a valid DOI and FALSE otherwise.
   */
  public static function isValid($doi) {
    return (bool) preg_match(static::REGEX, $doi);
  }

  /**
   * Counts DOI in 'chado.dbxref' table.
   *
   * @param string $doi
   *   DOI value.
   *
   * @return int
   *   Returns number of records for given DOI in 'chado.dbxref' table.
   *
   * @todo Minor. Find better name.
   */
  public static function countDbxref($doi) {
    return db_select('chado.dbxref', 'd')
      ->fields(NULL, ['accession'])
      ->condition('accession', $doi)
      ->countQuery()
      ->execute()
      ->fetchField();
  }

  /**
   * Searches for DOI in already submitted studies.
   *
   * @param string $doi
   *   DOI value.
   * @param bool $reset
   *   Flag is cache must be resetted. Default if FALSE.
   *
   * @return string
   *   Returns accession in format 'TGDRxxx'.
   *   Returns FALSE if DOI wasn't found.
   */
  public static function getAccession($doi, $reset = FALSE) {
    if (empty($doi)) {
      return FALSE;
    }
    $doi = strtolower($doi);
    $data = SubmissionCache::getAll($reset);
    foreach ($data as $accession => $cached_submission) {
      if (strtolower($cached_submission['publication']['publication_doi']) == $doi) {
        return $accession;
      }
    }
    return FALSE;
  }

  /**
   * Gets list of the Registration Agencies (RAs).
   *
   * See
   * https://www.doi.org/the-community/what-are-registration-agencies/ ;
   * https://www.doi.org/the-community/existing-registration-agencies ;
   */
  public static function getRegistrationAgencyList() {
    return [
      'Airiti DOI' => 'Taiwan-based RA for Chinese/English scholarly materials',
      'BSI Identify' => 'RA for construction products using UPINs',
      'Chinese DOI' => 'Chinese RA for journals, data, dissertations (ISTIC & Wanfang)',
      'CNKI' => 'China National Knowledge Infrastructure',
      'Crossref' => 'Global RA for scholarly research objects (19,000+ members)',
      'DataCite' => 'Global RA for research data and outputs',
      'EIDR' => 'Entertainment Identifier Registry (movies, TV, podcasts)',
      'JaLC' => 'Japan Link Center',
      'HAND' => 'Hollywood & sports talent identifier (Humans, Avatars, Characters)',
      'Korean DOI Center' => 'Korean RA for S&T info (KISTI)',
      'mEDRA' => 'multilingual European DOI Registration Agency',
      'OP' => 'Publications Office of the European Union',
    ];
  }

  /**
   * Gets list of DOI Registrants.
   *
   * Note:
   * Top-level DOI "10." is not a registrant (it's the directory indicator).
   *
   * @return array
   *   Returns list of registrants.
   */
  public static function getRegistrantList() {
    return [
      '10.1002' => 'Wiley (Publisher)',
      '10.1371' => 'Public Library of Science (PLOS)',
      '10.3389' => 'Frontiers',
      '10.5194' => 'Copernicus Publications',
      '10.1155' => 'Hindawi',
      '10.3897' => 'Pensoft Publishers',
      '10.5061' => 'Dryad (Data Repository)',
      '10.2172' => 'OSTI (Office of Scientific & Technical Information)',
      '10.3205' => 'German Medical Science',
      '10.3402' => 'Co-Action Publishing',

      // Major commercial publishers.
      '10.1016' => 'Elsevier',
      '10.1126' => 'AAAS (Science)',
      '10.1038' => 'Nature Publishing Group (Springer Nature)',
      '10.1093' => 'Oxford University Press (OUP)',
      '10.1111' => 'Wiley (Journals, secondary prefix)',
      '10.1186' => 'BioMed Central (Springer Nature)',
      '10.1242' => 'The Company of Biologists',
      '10.1245' => 'Society of Surgical Oncology',
      '10.1246' => 'CSJ (Chemical Society of Japan)',

      // Open access and society publishers.
      '10.5334' => 'Ubiquity Press',
      '10.5339' => 'QScience (Hamad Bin Khalifa University Press)',
      '10.7554' => 'eLife Sciences Publications',
      '10.7717' => 'PeerJ',
      '10.7910' => 'Harvard Dataverse',
      '10.7925' => 'Dublin City University (DCU)',
      '10.7930' => 'US Global Change Research Program (USGCRP)',

      // Data and research infrastructure.
      '10.4224' => 'Canadian Science Publishing (data)',
      '10.4225' => 'Griffith University (Data Repository)',
      '10.4230' => 'Schloss Dagstuhl – Leibniz-Zentrum für Informatik (LZI)',
      '10.5072' => 'Figshare (Test/Generic)',
      '10.5281' => 'Zenodo (CERN)',
      '10.5282' => 'Universitätsbibliothek der LMU München',
      '10.5283' => 'Freie Universität Berlin',

      // Official Registration Agencies (prefixes they administer).
      '10.1645' => 'Crossref (test/example DOI prefix)',
      '10.1445' => 'DataCite (BETA/example)',
      '10.2481' => 'JaLC (Japan Link Center)',
      '10.2490' => 'Korean Federation of Science and Technology Societies (KOFST)',
      '10.2500' => 'OceanSide Publications (via Crossref)',

      // Smaller / notable independent registrants.
      '10.1073' => 'PNAS (Proceedings of the National Academy of Sciences)',
      '10.1371' => 'PLOS (Public Library of Science)',
      '10.1371' => 'Public Library of Science (PLOS) [duplicate example]',
      '10.1385' => 'Humana Press (now part of Springer)',
      '10.1478' => 'CERN (High Energy Physics)',
      '10.2505' => 'National Science Teachers Association (NSTA)',

      // Additional academic presses.
      '10.7208' => 'University of Chicago Press',
      '10.7312' => 'Columbia University Press',
      '10.7591' => 'Cornell University Press',
      '10.2307' => 'JSTOR (books / early journal content)',
      '10.3998' => 'University of Michigan Press / MPublishing',
    ];
  }

}
