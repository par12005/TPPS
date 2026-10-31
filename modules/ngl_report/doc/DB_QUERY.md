## How to add rows counter

- Use addExpression() instead of addField() of fields().
- Do NOT Use ```['data' => '#', 'field' => '#']``` because sortring won't work.

Example:
```
function tpps_report_organism_gbif_missing() {
  $report_name = 'organism_gbif_missing';
  $source = GbifSpeciesApi::KEY;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $query = db_select('chado.organism', 't');
  //Get taxonomy id.
  $query->leftJoin(
    'public.' . OrganismTaxonomy::TABLE,
    'ot',
    't.organism_id = ot.organism_id AND ot.source = :source',
    [':source' => $source]
  );
  // Infraspecific Rank.
  $query->leftJoin(
    'chado.cvterm',
    'cvt',
    'cvt.cvterm_id = t.type_id AND cvt.cv_id = :cv_id',
    [':cv_id' => InfraspecificRank::CV_ID]
  );
  // Add the row number expression
  $query->addExpression('ROW_NUMBER() OVER ()', 'counter');
  // Drupal's SelectQuery builder (db_select) always renders table fields
  // (addField / fields) before calculated expressions (addExpression) when
  // compiling the SELECT clause, regardless of the order in which you call
  // the methods in PHP.
  $query->addExpression('t.organism_id', 'organism_id');
  $query->addExpression('t.genus', 'genus');
  $query->addExpression('t.species', 'species');
  $query->addExpression('cvt.name', 'name');
  $query->addExpression('t.infraspecific_name', 'infraspecific_name');

  $query->isNull('ot.taxonomy_id');

  $header = [
    ['data' => '#'],
    ['data' => t('Organism Id'), 'field' => 't.organism_id'],
    ['data' => t('Genus'), 'field' => 't.genus'],
    ['data' => t('Species'), 'field' => 't.species'],
    ['data' => t('Infraspecific Rank'), 'field' => 'cvt.name'],
    ['data' => t('Infraspecific Name'), 'field' => 't.infraspecific_name'],
  ];

  return Report::print([
    'name' => $report_name,
    'header' => $header,
    'query' => $query,
  ], $reset);
}

```
