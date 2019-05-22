<?php

/**
 * @file
 */

/**
 *
 */
function tpps_front_create_form(&$form, $form_state) {

  global $base_url;
  global $user;

  if (isset($user->mail)) {
    // Logged in.
    $options_arr = array();
    $options_arr['new'] = 'Create new TPPS Submission';

    $results = db_select("public.variable", "variable")
      ->fields('variable', array('name'))
      ->condition('name', db_like('tpps_incomplete_' . $user->mail) . '%', 'LIKE')
      ->execute();

    foreach ($results as $item) {
      $name = $item->name;
      $state = variable_get($name, NULL);

      if ($state != NULL and isset($state['saved_values'][TPPS_PAGE_1]['publication']['title'])) {
        $title = ($state['saved_values'][TPPS_PAGE_1]['publication']['title'] != NULL) ? $state['saved_values'][TPPS_PAGE_1]['publication']['title'] : "No Title";
        $tgdr_id = $state['accession'];
        $options_arr["$tgdr_id"] = "$title";
      }
      else {
        if (isset($state) and !isset($state['saved_values'][TPPS_PAGE_1])) {
          variable_del($name);
          $and = db_and()
            ->condition('accession', $state['accession'])
            ->condition('db_id', 95);
          $results = db_delete('chado.dbxref')
            ->condition($and)
            ->execute();
        }
      }
    }

    if (count($options_arr) > 1) {
      // Has submissions.
      $form['accession'] = array(
        '#type' => 'select',
        '#title' => t('Would you like to load an old TPPS submission, or create a new one?'),
        '#options' => $options_arr,
        '#default_value' => isset($form_state['saved_values']['frontpage']['accession']) ? $form_state['saved_values']['frontpage']['accession'] : 'new',
      );
    }
  }

  $form['Next'] = array(
    '#type' => 'submit',
    '#value' => t('Continue to TPPS'),
  );

  $prefix_text =
  "<div>
Welcome to TPPS!<br><br>
The Tripal PopGen Submit (TPPS) workflow provides researchers with a streamlined submission interface for studies resulting from any combination of genotype, phenotype, and environmental data for georeferenced forest trees.  TPPS will guide users through questions about their study design and data in order to collect information on trees, genotypes, and phenotypes if applicable.  Phenotypic, genotypic, and environmental descriptors will be mapped to ontologies where possible and the collected metadata will enable this information to be displayed in <a href='$base_url/cartogratree' target='blank'>CartograTree</a>.  An accession number will be provided to the user following successful completion that uniquely identifies this study in the database.  This number should be used in the manuscript describing this work.  Specific flat-files and metadata associated with this identifier can be accessed by the public with this information.<br><br>
To get started, you will need to have a few things handy:<br>
<ul>
  <li>An enabled and approved TreeGenes account - you can create one <a href='$base_url/user/register'>here</a>. There may be a waiting period to have your account approved by a TreeGenes administrator.</li>
  <li>Information about the paper connected to your study, and the organisms being studied. This must include at least the following:
    <ul>
      <li>Primary author of the publication</li>
      <li>Organization of the Primary Author</li>
      <li>Publication year</li>
      <li>Publication title</li>
      <li>Publication abstract</li>
      <li>Journal the publication can be found in</li>
      <li>Genus and species of the organism(s) being studied</li>
    </ul>
  </li>
  <li>Metadata about the study itself. This must include at least the following:
    <ul>
      <li>Dates when the study took place (month, year)</li>
      <li>The location of the study (coordinates or country/region)</li>
      <li>Type of study (natural population, growth chamber, common garden, etc.)</li>
      <li>Type of data collected (some combination of genotype, phenotype, and environmental)</li>
      <li>Relevant quantitative information based on the type of study, such as the seasons of a natural population study, or the soil pH of a growth chamber study</li>
    </ul>
  </li>
  <li>Geographic locations of the trees (for common garden studies, this would be the location of the source tree). This should be a spreadsheet with a column for tree identifiers and column(s) for the location of each tree.</li>
  <li>Genotypic and/or phenotypic data and metadata (depending on the type(s) of data collected).</li>
</ul>
If you would like to submit your data, you can click the button 'Continue to TPPS' below!<br><br>
</div>";

  if (isset($form['accession'])) {
    $form['accession']['#prefix'] = $prefix_text;
  }
  else {
    $form['Next']['#prefix'] = $prefix_text;
  }

  return $form;
}
