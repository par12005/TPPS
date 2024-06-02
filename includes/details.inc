<?php

/**
 * @file
 * Defines functions to be used in the TPPS Details pages.
 */

/**
 * Menu callback. Shows list of stydies.
 *
 * @return string
 *   The appropriate output.
 */
function tpps_details_list() {
  global $base_url;
  global $user;
  $output = "";

  // @TODO Cache those numbers.
  $query = db_select('chado.plusgeno_view', 'pg');
  $query->addExpression('count(distinct(accession))', 'count');
  $query = $query->execute();
  $num_studies = $query->fetchObject()->count;

  $query = db_select('chado.plusgeno_view', 'pg');
  $query->addExpression('count(distinct(species))', 'count');
  $query = $query->execute();
  $num_species = $query->fetchObject()->count;

  // @TODO Minor. Replace with l().
  $tpps_link = "<b><a target=\"blank\" href=\"/tpps\">submit your study</a></b>";
  $output .= "This page reflects community submissions associated with published studies. Each submission is associated with a longterm identifier that connects the genotype, phenotype, and/or environmental metrics assessed. These studies represent diversity assessements, association mapping, and landscape genomics. We accept studies with a variety of marker types. We encourage you to $tpps_link to TreeGenes and utilize the provided accession in your publication. We currently have <b>$num_studies</b> studies representing <b>$num_species</b> species.";
  if (module_exists('cartogratree')) {
    $output .= " Studies that have georeferenced accessions associated with them can be queried on <b><a href=\"cartogratree\">CartograPlant.</a></b>";
  }
  $form = drupal_get_form('tpps_details_search');
  tpps_add_css_js('main', $form);
  $output .= render($form);
  $output .= tpps_details_tags_sort();
  $output .= "<div id='tpps-details-table'>" . tpps_details_top() . "</div>";
  return $output;
}

/**
 * Starting point for most TPPS Details pages.
 *
 * Either returns the top details page, which allows users to select from
 * completed studies, or returns the per-submission details page, which allows
 * users to browse the details of a study.
 *
 * @param string $accession
 *   The accession to get details from. If NULL, show all submissions.
 *
 * @return string
 *   The appropriate output.
 */
function tpps_details($accession = NULL) {
  global $base_url;
  global $user;

  // @TODO This should never happen. Remove if possible.
  if (empty($accession)) {
    drupal_goto('tpps/details');
  }

  $output = l(t('Return to TPPS Details'), $base_url . '/tpps/details');
  $submission = new Submission($accession);
  if (!empty($submission->state)) {
    if (isset($submission->state['study_view_role'])) {
      // If more than 0, then a view role was set by curation team
      if ($submission->state['study_view_role'] > 0) {
        if (
          !in_array(user_roles($true)[$submission->state['study_view_role']], $user->roles)
          // @TODO Minor. Admins must always has access so this check could
          // be done earlier and avoid extra checks.
          && !in_array('administrator', $user->roles)
        ) {
          $output .= t("<br />This study has been set to be available to only "
            . "a specific user role. <br />If you believe this is an error, "
            . "please contact us via the contact us page.<br />");
          // @TODO Minor. Use drupal_set_message() and redirect to 'tpps/detals' page.
          return $output;
        }
      }
    }

    if ($submission->state['status'] != TPPS_SUBMISSION_STATUS_APPROVED) {
      drupal_set_message(
        t('This submission is still under construction. Some of the information '
        . 'may be available here, but please keep in mind that it may be '
        . 'incomplete or subject to change in the near future.'),
        'warning'
      );
      if (
        variable_get('tpps_details_use_external_tabs', FALSE)
        && function_exists('tpps_details_byaccession_page')
      ) {
        $output .= @tpps_details_byaccession_page()['raw_markup']['#markup'];
        tpps_add_css_js('main');
        return $output;
      }
    }

    $map_api_key = variable_get('tpps_maps_api_key', NULL);
    $map_api_tools = "<script src=\"https://developers.google.com/maps/documentation/javascript/examples/markerclusterer/markerclusterer.js\"></script><script src=\"https://maps.googleapis.com/maps/api/js?key=$map_api_key&callback=initMap\"
      async defer></script>"
      . "<div id=\"_map_wrapper\"></div>";
    $tree_info = [];
    if (!empty($submission->state['tree_info'])) {
      foreach ($submission->state['tree_info'] as $name => $info) {
        if (
          !empty($info['lat'])
          and !empty($info['lng'])
        ) {
          $tree_info[] = [
            $name,
            $info['lat'],
            $info['lng'],
          ];
        }
      }
      drupal_add_js(['tpps' => ['tree_info' => $tree_info]], 'setting');
    }
    $output .= $map_api_tools;
    // [VS].
    tpps_add_css_js('main');
    // [/VS].

    if (!empty($pub_link = ($submission->state['title'] ?? NULL))) {
      $pub_id = db_select('chado.project_pub', 'p')
        ->fields('p', array('pub_id'))
        ->condition('project_id', $submission->state['ids']['project_id'])
        ->execute()->fetchObject()->pub_id ?? NULL;
      if (!empty($pub_id)) {
        $bundle = tripal_load_bundle_entity(array('label' => 'Publication'));
        $pub_id = chado_get_record_entity_by_bundle($bundle, $pub_id);
        $pub_link = "<a href='$base_url/Publication/$pub_id'>$pub_link</a>";
      }
      $output .= "<h2>$pub_link</h2>";
    }
    $output .= tpps_show_tags(tpps_submission_get_tags($accession));
    if ($primary_author = $submission->state['authors'][0] ?? NULL) {
      if (preg_match('/,/', $primary_author)) {
        $parts = explode(',', $primary_author);
        $lastname = trim($parts[0]);
      }
      else {
        $parts = explode(' ', $primary_author);
        $lastname = $parts[count($parts) - 1];
      }
      $output .= "<h4>$lastname et al {$submission->state['pyear']}";
      if (!empty($submission->state['journal'])) {
        $output .= " published in {$submission->state['journal']}";
      }
      $output .= "</h4>";
      // @TODO Minor. Use l() and drupal_add_js().
      $output .= "<div id=\"abstract\"><a href=\"#abstract\">Click to show abstract</a></div>"
        . "<script>jQuery(document).ready(jQuery('#abstract').on('click', function() {
            jQuery('#abstract')[0].innerHTML = `{$submission->state['abstract']}`;
          }));</script>";
    }
    else {
      $output .= '<br />';
    }
    if (module_exists('cartogratree')) {
      $items[] = l(t('View in CartograPlant'), 'cartogratree',
        [
          'query' => ['accession' => $accession],
          'attributes' => ['target' => 'blank'],
        ]
      );
      $items[] = l(t('Download compressed raw files'),
        'tpps/api/files/' . $accession, ['attributes' => ['target' => 'blank']]
      );
    }

    $page1_values = &$submission->state['saved_values'][TPPS_PAGE_1];
    if (
      $page1_values['publication']['status'] != 'Published'
      && $submission->isCreator()
    ) {
      $items[] = l(t('Edit publication information'), 'tpps/' . $accession . '/edit-publication');
    }
    $output .= theme('item_list', ['items' => $items]);
    $output .= tpps_details_tabs($submission->state);
    return $output;
  }

  drupal_set_message(t('This submission is pending resubmission through the '
    . 'new TPPS pipeline. Some of the information may be available here, '
    . 'but please keep in mind that it may be incomplete or subject to '
    . 'change in the near future.'),
    'warning'
  );

  if (function_exists('tpps_details_byaccession_page')) {
    $output .= @tpps_details_byaccession_page()['raw_markup']['#markup'];
    // [VS].
    tpps_add_css_js('main');
    // [/VS].
  }
  return $output;
}

/**
 * Returns a template for details tabs.
 *
 * To add new tab - just add new element to $tab_list array.
 * Note that all links in content will be replaced with current path and
 * fragment so update /js/tpps.js if you don't want that. Search 'submission'.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   HTML for the details tabs.
 */
function tpps_details_tabs(array &$state) {
  $page4 = $state['saved_values'][TPPS_PAGE_4] ?? NULL;

  $tab_items = [];
  $content = [];
  $tabs_list = [
    'species' => 'Species',
    'study' => 'Study Details',
    'trees' => 'Plants',
  ];
  for ($i = 1; $i <= $state['stats']['species_count']; $i++) {
    if (!empty($page4["organism-$i"]['phenotype'])) {
      $tabs_list['phenotype'] = 'Phenotypes / Environments';
    }
    if (!empty($page4["organism-$i"]['genotype'])) {
      $tabs_list['genotype'] = 'Genotypes';
    }
    if (
      !empty($page4["organism-$i"]['phenotype'])
      && !empty($page4["organism-$i"]['phenotype']['iso-check'])
    ) {
      $tabs_list['mass_spectrometry'] = 'Mass Spectrometry';
    }
  }
  // Build HTML.
  // Only first element must have class 'active'.
  $content_class = 'tab-pane fade in active';
  foreach ($tabs_list as $key => $title) {
    $tab_items[] = [
      'data' => l(t($title), '',
      [
        'external' => TRUE,
        'fragment' => $key,
        'attributes' => [
          'class' => ['nav-link'],
          'role' => 'tab',
          'data-toggle' => 'tab',
        ],
      ]),
      'class' => ['nav-item'],
    ];
    $content[] = '<div id="' . $key . '" role="tabpanel" '
      . 'class="' . $content_class . '"></div>';
    $content_class = 'tab-pane fade';
  }
  return theme('item_list', [
    'items' => $tab_items,
    'type' => 'ul',
    'attributes' => [
      'class' => ['nav', 'nav-tabs'],
      'role' => 'tablist',
    ],
  ]) . '<div id="tab-content" class="tab-content">' . implode('', $content) . '</div>';
}

/**
 * A callback to print a single detail tab.
 *
 * @param string $accession
 *   The accession number of the submission.
 * @param string $type
 *   The type of details we want to print.
 */
function tpps_details_tab($accession, $type) {
  $function = "tpps_details_$type";
  $submission = new Submission($accession);
  print($function($submission->sharedState));
}

/**
 * Builds the TPPS Details page when no accession is provided.
 *
 * @return string
 *   The HTML string for the TPPS Details page.
 *
 * @TODO [VS] Minor. This page was broken. Seems not used and outdated. Remove it.
 */
function tpps_details_top() {
  global $base_url;
  $params = drupal_get_query_parameters($_POST);
  $page = 0;
  if (empty($params)) {
    $params = drupal_get_query_parameters();
  }
  if (!empty($params['page'])) {
    $page = $params['page'];
  }
  $per_page = 20;

  $query = db_select('chado.plusgeno_view', 'pg');
  $query->addExpression('count(distinct(pg.accession))', 'count');
  // $query->addExpression('substring(pg.accession,5)', 'accession_number');
  if (!empty($params['type']) and !empty($params['value']) and !empty($params['op'])) {
    switch ($params['type']) {
      case 'title':
      case 'species':
      case 'project_id':
      case 'accession':
      case 'author':
      case 'year':
        $query->condition($params['type'], $params['value'], $params['op']);
        break;

      case 'phenotype_name':
        $query->innerJoin('chado.project_stock', 'ps', 'ps.project_id = pg.project_id');
        $query->innerJoin('chado.stock_phenotype', 'sp', 'sp.stock_id = ps.stock_id');
        $query->innerJoin('chado.phenotype', 'ph', 'ph.phenotype_id = sp.phenotype_id');
        $query->condition('ph.name', $params['value'], $params['op']);
        break;

      case 'phenotype_ontology':
        $query->innerJoin('chado.project_stock', 'ps', 'ps.project_id = pg.project_id');
        $query->innerJoin('chado.stock_phenotype', 'sp', 'sp.stock_id = ps.stock_id');
        $query->innerJoin('chado.phenotype', 'ph', 'ph.phenotype_id = sp.phenotype_id');
        $query->innerJoin('chado.cvterm', 'cvt', 'cvt.cvterm_id = ph.attr_id');
        $query->innerJoin('chado.cv', 'cv', 'cv.cv_id = cvt.cv_id');
        $query->condition('cv.name', $params['value'], $params['op']);
        break;

      case 'genotype_name':
        $query->innerJoin('chado.tpps_search_genotype_name', 'g', 'g.project_id = pg.project_id');
        $query->condition('g.name', $params['value'], $params['op']);
        break;

      case 'genotype_marker':
        $query->innerJoin('chado.tpps_search_genotype_marker', 'g', 'g.project_id = pg.project_id');
        $query->condition('g.name', $params['value'], $params['op']);
        break;

      case 'tags':
        $query->innerJoin('public.tpps_submission', 'ts', 'ts.accession = pg.accession');
        $query->innerJoin('public.tpps_submission_tag', 'st', 'st.tpps_submission_id = ts.tpps_submission_id');
        $query->innerJoin('public.tpps_tag', 'tt', 'tt.tpps_tag_id = st.tpps_tag_id');
        $query->condition('tt.name', $params['value'], $params['op']);
        break;

      default:
        break;
    }
  }

  $query = $query->execute();
  $total = $query->fetchObject()->count;

  $_GET['page'] = $page;
  $page = pager_default_initialize($total, $per_page);
  $start = $page * $per_page;

  $submissions = db_select('chado.plusgeno_view', 'pg');
  $submissions->distinct();
  $submissions->innerJoin('chado.project', 'p', 'p.project_id = pg.project_id');
  if (!empty($params['type']) and !empty($params['value']) and !empty($params['op'])) {
    switch ($params['type']) {
      case 'title':
      case 'species':
      case 'project_id':
      case 'accession':
      case 'author':
      case 'year':
        $submissions->condition($params['type'], $params['value'], $params['op']);
        break;

      case 'phenotype_name':
        $submissions->innerJoin('chado.project_stock', 'ps', 'ps.project_id = pg.project_id');
        $submissions->innerJoin('chado.stock_phenotype', 'sp', 'sp.stock_id = ps.stock_id');
        $submissions->innerJoin('chado.phenotype', 'ph', 'ph.phenotype_id = sp.phenotype_id');
        $submissions->condition('ph.name', $params['value'], $params['op']);
        break;

      case 'phenotype_ontology':
        $submissions->innerJoin('chado.project_stock', 'ps', 'ps.project_id = pg.project_id');
        $submissions->innerJoin('chado.stock_phenotype', 'sp', 'sp.stock_id = ps.stock_id');
        $submissions->innerJoin('chado.phenotype', 'ph', 'ph.phenotype_id = sp.phenotype_id');
        $submissions->innerJoin('chado.cvterm', 'cvt', 'cvt.cvterm_id = ph.attr_id');
        $submissions->innerJoin('chado.cv', 'cv', 'cv.cv_id = cvt.cv_id');
        $submissions->condition('cv.name', $params['value'], $params['op']);
        break;

      case 'genotype_name':
        $submissions->innerJoin('chado.tpps_search_genotype_name', 'g', 'g.project_id = pg.project_id');
        $submissions->condition('g.name', $params['value'], $params['op']);
        break;

      case 'genotype_marker':
        $submissions->innerJoin('chado.tpps_search_genotype_marker', 'g', 'g.project_id = pg.project_id');
        $submissions->condition('g.name', $params['value'], $params['op']);
        break;

      case 'tags':
        $submissions->innerJoin('public.tpps_submission', 'ts', 'ts.accession = pg.accession');
        $submissions->innerJoin('public.tpps_submission_tag', 'st', 'st.tpps_submission_id = ts.tpps_submission_id');
        $submissions->innerJoin('public.tpps_tag', 'tt', 'tt.tpps_tag_id = st.tpps_tag_id');
        $submissions->condition('tt.name', $params['value'], $params['op']);
        break;

      default:
        break;
    }
  }
  $submissions->fields('pg', array(
    'title',
    'project_id',
    'accession',
    'tree_count',
    'phenotypes_assessed',
    'phen_count',
    'gen_count',
  ));
  $submissions->addExpression('SUBSTRING(pg.accession, 5)::integer', 'accession_number_only');
  $submissions->range($start, $per_page);
  // $submissions->orderBy('pg.accession');
  $submissions->orderBy('accession_number_only', 'ASC');
  $submissions = $submissions->execute();

  $rows = array();
  while (($sub = $submissions->fetchObject())) {
    $proj_id = $sub->project_id;

    $query = db_select('chado.organism', 'o');
    $query->join('chado.pub_organism', 'po', 'o.organism_id = po.organism_id');
    $query->join('chado.project_pub', 'pp', 'pp.pub_id = po.pub_id');
    $query->fields('o', array('organism_id', 'genus', 'species'));
    $query->condition('pp.project_id', $proj_id);
    $query->distinct();
    $query = $query->execute();

    $species = array();
    while (($result = $query->fetchObject())) {
      $species[] = tpps_entity_link($result->organism_id, "{$result->genus} {$result->species}", 'Organism');
    }

    $warning = "";
    $submission = new Submission($sub->accession);
    if ($submission->doesNotExist()) {
      $title = t('This study has not yet been resubmitted through the new TPPS pipeline');
      $warning = "<img src='$base_url/misc/message-16-warning.png' title='"
        . $title . "'> ";
    }

    $row = array(
      "<a href=\"$base_url/tpps/details/{$sub->accession}\">{$sub->accession}</a>",
      "$warning<a href=\"$base_url/tpps/details/{$sub->accession}\">{$sub->title}</a>",
      tpps_show_tags(tpps_submission_get_tags($sub->accession)),
      implode('<br>', $species),
      $sub->tree_count,
      $sub->phenotypes_assessed,
      $sub->phen_count,
      $sub->gen_count,
    );
    $rows[$sub->accession] = $row;
  }

  ksort($rows);

  $vars = array(
    'header' => array(
      'Accession',
      'Title',
      'Tags',
      'Species',
      'Plant Count',
      'Phenotypes Assessed',
      'Phenotypic Measures',
      'Genotype Count',
    ),
    'rows' => $rows,
    'attributes' => array(
      'class' => array('view'),
      'id' => 'tpps_table_display',
    ),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );

  $output = theme('table', $vars);
  $pager = theme('pager', array('quantity', $total));
  return $pager . $output;
}

/**
 * Serves as a direct callback to print the contents of tpps_details_top().
 */
function tpps_details_top_callback() {
  print(tpps_details_top());
}

/**
 * Define a search form to search and filter through the TPPS Details page.
 *
 * @param array $form
 *   The form array object.
 * @param array $form_state
 *   The form state array object.
 *
 * @return array
 *   The populated search form.
 */
function tpps_details_search(array $form, array $form_state) {
  $params = drupal_get_query_parameters();
  $form['details_type'] = array(
    '#type' => 'select',
    '#options' => array(
      'title' => t('Title'),
      'species' => t('Species'),
      'accession' => t('Accession'),
      'author' => t('Primary Author'),
      'year' => t('Publication Year'),
      'phenotype_name' => t('Phenotype Name'),
      'phenotype_ontology' => t('Phenotype Ontology Name'),
      'genotype_name' => t('Genotype Name'),
      'genotype_marker' => t('Genotype Marker Type'),
      'tags' => t('Tags'),
    ),
    '#ajax' => array(
      'wrapper' => 'tpps-details-form',
      'callback' => 'tpps_details_form_callback',
    ),
    '#prefix' => '<div id="tpps-details-form">',
    '#default_value' => $params['type'] ?? NULL,
  );

  $ops = array(
    '~*' => '~*',
    'LIKE' => 'LIKE',
    '=' => '=',
  );

  $form['details_op'] = array(
    '#type' => 'select',
    '#options' => $ops,
    '#default_value' => $params['op'] ?? NULL,
  );

  $form['details_value'] = array(
    '#type' => 'textfield',
    '#suffix' => '</div>',
    '#autocomplete_path' => 'tpps/autocomplete/project_title',
    '#default_value' => $params['value'] ?? NULL,
  );

  if (empty($form_state['values']['details_type'])) {
    $form_state['values']['details_type'] = $params['type'] ?? NULL;
  }

  if (!empty($form_state['values']['details_type'])) {
    switch ($form_state['values']['details_type']) {
      case 'title':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/project_title';
        break;

      case 'species':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/species';
        break;

      case 'accession':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/project_accession';
        break;

      case 'author':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/author';
        break;

      case 'phenotype_name':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/phenotype';
        break;

      case 'phenotype_ontology':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/phenotype_ontology';
        break;

      case 'genotype_name':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/genotype';
        break;

      case 'genotype_marker':
        $form['details_value']['#autocomplete_path'] = 'tpps/autocomplete/genotype_marker';
        break;

      default:
        $form['details_value']['#autocomplete_path'] = NULL;
        break;
    }
  }

  $form['details_search'] = array(
    '#type' => 'button',
    '#button_type' => 'button',
    '#value' => t('Search'),
  );

  $form['reset_button'] = array(
    '#markup' => '<button class="btn btn-primary form-button" type="reset"/>Reset</button>',
  );

  $form['#attributes'] = array(
    'style' => 'text-align:center',
  );

  return $form;
}

/**
 * Display TPPS tags to sort Details page.
 *
 * @return string
 *   Tag options as rendered HTML.
 */
function tpps_details_tags_sort() {
  $output = "";

  $query = db_select('tpps_tag', 't')
    ->fields('t')
    ->orderBy('static', 'DESC')
    ->orderBy('tpps_tag_id')
    ->execute();
  $tags = array();
  while (($result = $query->fetchObject())) {
    $tags[$result->tpps_tag_id] = array(
      'id' => $result->tpps_tag_id,
      'name' => $result->name,
      'color' => $result->color,
      'static' => $result->static,
    );
  }

  $output .= tpps_show_tags($tags);

  return "<label for=\"tpps-tags-filter\">Sort by tags:</label><div id=\"tpps-tags-filter\">" . $output . "</div>";
}

/**
 * AJAX callback for the search form details_op element.
 *
 * @param array $form
 *   The form being updated.
 * @param array $form_state
 *   The state of the form being updated.
 *
 * @return array
 *   The part of the form to be updated.
 */
function tpps_details_form_callback(array $form, array $form_state) {
  return array(
    'details_type' => $form['details_type'],
    'details_op' => $form['details_op'],
    'details_value' => $form['details_value'],
  );
}

/**
 * Creates the table for species details for a submission.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   The HTML table of species associated with the submission.
 */
function tpps_details_species(array &$state) {
  $query = db_select('chado.organism', 'o');
  $query->join('chado.pub_organism', 'po', 'o.organism_id = po.organism_id');
  $query->join('chado.project_pub', 'pp', 'pp.pub_id = po.pub_id');
  $query->fields('o', array('organism_id', 'genus', 'species', 'common_name'));
  $query->condition('pp.project_id', $state['ids']['project_id']);
  $query = $query->execute();
  $rows = array();

  while (($result = $query->fetchObject())) {
    $id = $result->organism_id;
    $common_name = $result->common_name;
    $family = $order = "";

    if (tpps_chado_prop_exists('organism', $id, 'family')) {
      $family = db_select('chado.organismprop', 'o')
        ->fields('o', array('value'))
        ->condition('organism_id', $id)
        ->condition('type_id', tpps_load_cvterm('family')->cvterm_id)
        ->execute()->fetchObject()->value;
    }

    if (tpps_chado_prop_exists('organism', $id, 'order')) {
      $order = db_select('chado.organismprop', 'o')
        ->fields('o', array('value'))
        ->condition('organism_id', $id)
        ->condition('type_id', tpps_load_cvterm('order')->cvterm_id)
        ->execute()->fetchObject()->value;
    }

    if (empty($common_name) and tpps_chado_prop_exists('organism', $id, 'common name')) {
      $common_name = db_select('chado.organismprop', 'o')
        ->fields('o', array('value'))
        ->condition('organism_id', $id)
        ->condition('type_id', tpps_load_cvterm('common name')->cvterm_id)
        ->execute()->fetchObject()->value;
    }

    $name = "{$result->genus} {$result->species}";
    $link = tpps_entity_link($id, $name, 'Organism');
    $rows[$name] = array(
      $order,
      $family,
      $link,
      $common_name,
    );
  }

  $vars = array(
    'header' => array(
      'Order',
      'Family',
      'Species',
      'Common Name',
    ),
    'rows' => $rows,
    'attributes' => array(
      'class' => array('view'),
      'id' => 'tpps_table_display',
    ),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );
  return theme('table', $vars);
}

/**
 * Creates the table for study information details for a submission.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   The HTML table of study information of the submission.
 *
 * @TODO Seems function not in use, probably outdated and could be removed.
 */
function tpps_details_study(array &$state) {
  $rows = array();

  $species = array();
  for ($i = 1; $i <= $state['stats']['species_count']; $i++) {
    $id = $state['ids']['organism_ids'][$i];
    $species_name = $state['saved_values'][TPPS_PAGE_1]['organism'][$i]['name'];
    $species[] = tpps_entity_link($id, $species_name, 'Organism');
  }
  $files = [];
  $study_file_ids = tpps_details_get_study_file_ids($state);
  // foreach ($state['files'] as $fid) {
  foreach ($study_file_ids as $fid) {
    if ($file = tpps_file_load($fid)) {
      $file_url = check_plain(file_create_url($file->uri));
      if (stripos($file_url, 'core/labs') === FALSE) {
        $files[] = "<a href='$file_url' target='blank'>$file->filename</a>";
      }
    }
  }
  $page2_values = $state['saved_values'][TPPS_PAGE_2]['study_type'] ?? NULL;

  $pub_link = $state['title'];
  $pub_id = db_select('chado.project_pub', 'p')
    ->fields('p', array('pub_id'))
    ->condition('project_id', $state['ids']['project_id'])
    ->execute()->fetchObject()->pub_id;
  if (!empty($pub_id)) {
    $bundle = tripal_load_bundle_entity(array('label' => 'Publication'));
    $pub_id = chado_get_record_entity_by_bundle($bundle, $pub_id);
    $pub_link = "<a href='$base_url/Publication/$pub_id'>$pub_link</a>";
  }
  $rows[] = array('Accession', $state['accession']);
  $rows[] = array('Title', $pub_link);
  $rows[] = array('Authors', implode(', ', $state['authors']));
  $rows[] = array('Species', implode('<br>', $species));
  $rows[] = array('Data Type', $page2_values['data_type']);
  module_load_include('inc', 'tpps', 'includes/form');
  $rows[] = ['Study Type', tpps_form_get_study_type($page2_values['study_type'])];
  $rows[] = array('File Downloads', implode('<br>', $files));
  $rows[] = array('Population Size', $state['stats']['tree_count']);
  if (!empty($state['stats']['phenotype_count'])) {
    $rows[] = array('Phenotype Count', $state['stats']['phenotype_count']);
  }
  if (!empty($state['stats']['unique_phenotypes'])) {
    $rows[] = array('Unique Phenotypes', $state['stats']['unique_phenotypes']);
  }
  // @todo genotype counts.
  // $rows[] = array('Genotype Count', $state['stats']['genotype_count']);
  $vars = array(
    'header' => array(),
    'rows' => $rows,
    'attributes' => array(
      'class' => array('view'),
      'id' => 'tpps_table_display',
    ),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );
  return theme('table', $vars);
}

/**
 * Gets list of study's File Ids.
 *
 * @param array $state
 *   Drupal Form State Array of any version.
 *
 * @return array
 *   Returns list of File Ids.
 *
 * @todo Minor. Use tpps_submission_file_get_parents().
 * @todo Probably not in use and outdated. Remove if possible.
 * (Called by orphan function tpps_details_study()).
 */
function tpps_details_get_study_file_ids(array $state) {

  $submission = new Submission();
  $submission->generateSharedState($state);
  $submission->save();
  $saved_values = &$submission->sharedState['saved_values'];
  // print_r($saved_values);
  $file_ids = [];
  $page4_file_list = [
    // Phenotype files.
    ['phenotype', 'file'],
    ['phenotype', 'metadata'],
    // Genotype files.
    ['genotype', 'files', 'snps-assay'], // Shared State.
    ['genotype', 'files', 'snps-association'],
    ['genotype', 'files', 'vcf'],
    ['genotype', 'files', 'ssrs'],
    ['genotype', 'files', 'ssrs_extra'],
    // What about 'Other spreadsheet' field?
    // ['genotype', 'files', 'other'],
  ];
  for ($j = 1; $j <= ($saved_values[TPPS_PAGE_1]['organism']['number'] ?? 1); $j++) {
    // Page 3.
    $file_ids[] = $saved_values[TPPS_PAGE_3]['tree-accession']['species-' . $j]['file'] ?? NULL;
    // Page 4.
    foreach ($page4_file_list as $parents) {
      $file_ids[] = drupal_array_get_nested_value(
        $saved_values[TPPS_PAGE_4]['organism-' . $j],
        $parents
      );
    }
  }

  // Supplemental files at 'summarypage'.
  for ($i = 1; $i <= 10; $i++) {
    $file_ids[] = $saved_values['summarypage']['files'][$i];
  }

  // Remove empty elements.
  return array_filter($file_ids);
}

/**
 * Creates the table of individual plant details for a submission.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   The HTML table of plants associated with the submission.
 */
function tpps_details_trees(array &$state) {
  $path = current_path();
  $args = explode('/', $path);
  $params = drupal_get_query_parameters($_POST);
  $page = 0;
  if (!empty($params['page'])) {
    $page = $params['page'];
  }
  $rows = array();
  $per_page = 20;

  if (empty($state['tree_info'])) {
    $output = t('The plant information for this study has not been properly updated. Please contact your site administrator.');
    return $output;
  }

  $_GET['page'] = $page;
  pager_default_initialize(count($state['tree_info']), $per_page);
  $chunks = array_chunk($state['tree_info'], $per_page, TRUE);

  foreach ($chunks[$page] as $name => $info) {
    $id = $info['organism_id'];
    $form_number = array_search($id, $state['ids']['organism_ids']);
    $species_name = $state['saved_values'][TPPS_PAGE_1]['organism'][$form_number]['name'];

    $tree_id = tpps_entity_link($info['stock_id'], $name, 'Stock');

    $row = array(
      strtoupper($args[2]) . '-' . strip_tags($tree_id),
      tpps_entity_link($info['stock_id'], $name, 'Stock'),
      tpps_entity_link($id, $species_name, 'Organism'),
    );
    $row[] = $info['lat'];
    $row[] = $info['lng'];
    $rows[] = $row;
  }

  $header = [
    t('TreeGenes Identifier'),
    t('Identifier'),
    t('Species'),
  ];
  //if (!$skip) {
    $header[] = t('Latitude');
    $header[] = t('Longitude');
  //}

  $vars = [
    'header' => $header,
    'rows' => $rows,
    'attributes' => ['class' => ['view'], 'id' => 'tpps_table_display'],
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  ];
  $table = theme('table', $vars);
  $pager = theme('pager', array('quantity', count($state['tree_info'])));
  return $pager . $table;
}

/**
 * Creates the table of individual plant phenotype details for a submission.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   The HTML table of plants and phenotypes associated with the submission.
 */
function tpps_details_phenotype(array &$state) {
  $params = drupal_get_query_parameters($_POST);
  $page = 0;
  if (!empty($params['page'])) {
    $page = $params['page'];
  }
  $rows = array();
  $per_page = 20;
  $start = $page * $per_page;

  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  $query->addExpression('count(p.phenotype_id)', 'count');
  $query->condition('ps.project_id', $state['ids']['project_id']);
  $query = $query->execute();
  $total = $query->fetchObject()->count;

  $_GET['page'] = $page;
  pager_default_initialize($total, $per_page);

  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  $query->fields('p', array(
    'phenotype_id',
    'name',
    'attr_id',
    'observable_id',
    'value',
  ));
  $query->fields('s', array('uniquename', 'stock_id'));
  $query->condition('ps.project_id', $state['ids']['project_id']);
  $query->range($start, $per_page);
  $query = $query->execute();

  // $query = db_select('chado.new_pheno_view_update', 'p');
  // $query->addField('p', 'phenotype_id', 'phenotype_id');
  // $query->addField('p', 'synonym', 'name');
  // $query->addField('p', 'structure_id', 'attr_id');
  // $query->addField('p', 'observable_id', 'observable_id');
  // $query->addField('p', 'value', 'value');
  // $query->addField('p', 'tree_acc', 'uniquename');
  // $query->addField('p', 'stock_id', 'stock_id');
  // $query->condition('p.project_id', $state['ids']['project_id']);
  // $query->range($start, $per_page);
  // $query = $query->execute();

  while (($result = $query->fetchObject())) {
    // preg_match('/^TGDR...-(.*)$/', $result->uniquename, $matches);
    preg_match('/^TGDR.{1,4}-(.*)$/', $result->uniquename, $matches);
    $tree = $matches[1];
    $obs = chado_get_cvterm(array(
      'cvterm_id' => $result->observable_id,
    ))->name ?? "";
    $attr = chado_get_cvterm(array(
      'cvterm_id' => $result->attr_id,
    ))->name ?? "";

    $rows[] = array(
      tpps_entity_link($result->stock_id, $tree, 'Stock'),
      tpps_entity_link($result->phenotype_id, $result->name, 'Phenotype'),
      $attr,
      $obs,
      $result->value,
    );
  }

  $table = theme('table', [
    'header' => [
      t('Plant'),
      t('Name'),
      t('Attribute'),
      t('Entity'),
      t('Value'),
    ],
    'rows' => $rows,
    'attributes' => ['class' => ['view'], 'id' => 'tpps_table_display'],
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  ]);
  $pager = theme('pager', ['quantity', $total]);
  return $pager . $table;
}

/**
 * Creates the table of individual plant genotype details for a submission.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   The HTML table of plants and genotypes associated with the submission.
 */
function tpps_details_genotype(array &$state) {
  $project_id = $state['ids']['project_id'];
  $params = drupal_get_query_parameters($_POST);
  $page = 0;
  if (!empty($params['page'])) {
    $page = $params['page'];
  }
  $rows = array();
  $per_page = 20;
  $start = $page * $per_page;

  // // Set total check = false to begin with
  // $total_check = false;

  // // Check cache to see if a total is available
  // $total_array = variable_get('tpps_tab_genotype_' + $state['ids']['project_id'] + '_total');
  // if($total_array == NULL) {
  //   $total_check = true;
  // }
  // else {
  //   // total array should have an expire_time
  //   if($total_array['expire_time'] < time()) {
  //     $total_check = true;
  //   }
  //   else {
  //     $total = $total_array['total'];
  //   }

  // }

  //

  // $dev = FALSE;
  // if($dev == TRUE) {
  //   $total_check = true;
  // }

  // if($total_check) {
  //   $query = db_select('chado.genotype', 'g');
  //   $query->join('chado.stock_genotype', 'sg', 'sg.genotype_id = g.genotype_id');
  //   $query->join('chado.project_stock', 'ps', 'ps.stock_id = sg.stock_id');
  //   $query->addExpression('count(g.genotype_id)', 'count');
  //   $query->condition('ps.project_id', $state['ids']['project_id']);
  //   $query = $query->execute();
  //   $total = $query->fetchObject()->count;

  //   $total_array = array(
  //     'expire_time' => time() + variable_get('tpps_tab_genotype_total_cache_time', 7*(24*60*60)),
  //     'total' => $total
  //   );
  //   variable_set('tpps_tab_genotype_' + $state['ids']['project_id'] + '_total', $total_array);
  // }

  // VERSION 2 - RISH - better but far from perfect - can get as slow a 1 minute per query
  // $query_results = chado_query('SELECT COUNT(*) as c1 FROM chado.genotypes_' . $project_id);

  // @NEW -RISH using Emily's redesign that uses index_id. Fast response in ms.
  $query_results = chado_query('select max(index_id) as c1 FROM chado.genotypes_' . $project_id,[]);

  $total = 0;
  foreach ($query_results as $row) {
    $total = $row->c1;
  }


  $_GET['page'] = $page;
  pager_default_initialize($total, $per_page);

  // Removed by Rish in favour of materialized views per project_id / study 3/24/2023
  // $query = db_select('chado.genotype', 'g');
  // $query->join('chado.stock_genotype', 'sg', 'sg.genotype_id = g.genotype_id');
  // $query->join('chado.project_stock', 'ps', 'ps.stock_id = sg.stock_id');
  // $query->join('chado.stock', 's', 's.stock_id = sg.stock_id');
  // $query->fields('g', array(
  //   'genotype_id',
  //   'name',
  //   'uniquename',
  //   'description',
  //   'type_id',
  // ));
  // $query->fields('s', array('uniquename', 'stock_id'));
  // $query->condition('ps.project_id', $state['ids']['project_id']);
  // $query->range($start, $per_page);
  // $query = $query->execute();

  // VERSION 2 - RISH - Faster but still linearly slower
  // $query = chado_query("SELECT * FROM chado.genotypes_" . $project_id . " OFFSET $start LIMIT $per_page");

  // @NEW - RISH using Emily's index_id to paginate the page columns
  $query = chado_query("select * from chado.genotypes_" . $project_id .
    " where index_id > " . $start .
    " order by index_id asc
    limit " . $per_page . ";",[]);

  while (($result = $query->fetchObject())) {
    // preg_match('/^TGDR...-(.*)$/', $result->s_uniquename, $matches);
    preg_match('/^TGDR.{1,4}-(.*)$/', $result->s_uniquename, $matches);
    $tree = $matches[1];
    $type = chado_get_cvterm(array(
      'cvterm_id' => $result->type_id,
    ))->name ?? "";
    $rows[] = array(
      tpps_entity_link($result->stock_id, $tree, 'Stock'),
      // tpps_entity_link($result->genotype_id, $result->uniquename, 'Genotype'),
      $result->name,
      $result->description,
      $type,
    );
  }

  $table = theme('table', [
    'header' => [
      t('Plant'),
      t('Name'),
      t('Description'),
      t('Marker Type'),
    ],
    'rows' => $rows,
    'attributes' => ['class' => ['view'], 'id' => 'tpps_table_display'],
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  ]);
  $pager = theme('pager', ['quantity', $total]);
  return $pager . $table;
}


/**
 * Creates the table for mass spectrometry details for a submission.
 *
 * @param array $state
 *   The form state of the submission.
 *
 * @return string
 *   The HTML table of plants and genotypes associated with the submission.
 */
function tpps_details_mass_spectrometry(array &$state) {
  $params = drupal_get_query_parameters($_POST);
  $page = 0;
  if (!empty($params['page'])) {
    $page = $params['page'];
  }
  $rows = array();
  $per_page = 20;
  $start = $page * $per_page;

  // $query = db_select('chado.phenotype', 'p');
  // $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  // $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  // $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  // $query->addExpression('count(p.phenotype_id)', 'count');
  // $query->condition('ps.project_id', $state['ids']['project_id']);
  // $query = $query->execute();
  // $total = $query->fetchObject()->count;

  // Get total count which is used for pager system (Rish)
  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  $query->addExpression('count(p.phenotype_id)', 'count');
  $query->condition('p.attr_id', tpps_load_cvterm('intensity')->cvterm_id);
  $query->condition('ps.project_id', $state['ids']['project_id']);
  $query = $query->execute();
  $total = $query->fetchObject()->count;

  $_GET['page'] = $page;
  pager_default_initialize($total, $per_page);

  $query = db_select('chado.phenotype', 'p');
  $query->join('chado.stock_phenotype', 'sp', 'sp.phenotype_id = p.phenotype_id');
  $query->join('chado.project_stock', 'ps', 'ps.stock_id = sp.stock_id');
  $query->join('chado.stock', 's', 's.stock_id = sp.stock_id');
  $query->fields('p', array(
    'phenotype_id',
    'name',
    'attr_id',
    'observable_id',
    'value',
  ));
  $query->fields('s', array('uniquename', 'stock_id'));
  $query->condition('p.attr_id', tpps_load_cvterm('intensity')->cvterm_id);
  $query->condition('ps.project_id', $state['ids']['project_id']);
  $query->range($start, $per_page);
  $query = $query->execute();

  while (($result = $query->fetchObject())) {
    preg_match('/^TGDR...-(.*)$/', $result->uniquename, $matches);
    $tree = $matches[1];
    $obs = chado_get_cvterm(array(
      'cvterm_id' => $result->observable_id,
    ))->name ?? "";
    $attr = chado_get_cvterm(array(
      'cvterm_id' => $result->attr_id,
    ))->name ?? "";

    $rows[] = array(
      tpps_entity_link($result->stock_id, $tree, 'Stock'),
      tpps_entity_link($result->phenotype_id, $result->name, 'Phenotype'),
      $attr,
      $obs,
      $result->value,
    );
  }

  $vars = array(
    'header' => array(
      'Plant',
      'Name',
      'Attribute',
      'Entity',
      'Value',
    ),
    'rows' => $rows,
    'attributes' => array(
      'class' => array('view'),
      'id' => 'tpps_table_display',
    ),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );
  $table = theme('table', $vars);
  $pager = theme('pager', array('quantity', $total));
  return $pager . $table;
}
