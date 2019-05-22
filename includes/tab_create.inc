<?php

/**
 * @file
 */

/**
 *
 */
function tpps_tab_create($type, $user, $name = NULL) {

  global $base_url;
  $email = $user->mail;
  $state_name = 'tpps_incomplete_' . $email;
  $params = drupal_get_query_parameters();
  $final = "";
  $t1 = $t2 = FALSE;

  if (isset($params['accession']) and isset($params['action']) and $params['action'] === 'DELETE') {
    $delete_name = $state_name . $params['accession'];
    $results = db_select("public.variable", "variable")
      ->fields('variable')
      ->condition('name', db_like($delete_name), 'LIKE')
      ->execute()
      ->fetchAssoc();

    variable_del($results['name']);
    $and = db_and()
      ->condition('accession', $params['accession'])
      ->condition('db_id', 95);
    $results = db_delete('chado.dbxref')
      ->condition($and)
      ->execute();

    // dpm($results);
  }

  $results = db_select("public.variable", "variable")
    ->fields('variable')
    ->condition('name', db_like($state_name) . '%', 'LIKE')
    ->execute();

  $style =
  '<style>
    .tpps_profile_tab {
      background: #ffffff;
      border: 1px solid #cccccc;
      border-radius: 4px;
      padding: 0px 10px;
    }
  </style>';
  $display = "<label for='tpps_incomplete_table'>Incomplete Submissions</label><div class='tpps_profile_tab'><table id='tpps_incomplete_table' style='width:-webkit-fill-available' border='1'><thead>";
  $display .= "<tr><th>Title</th><th>Primary Author</th><th>Number of Species</th><th>Abstract</th><th>Journal</th><th></th></tr>";
  $display .= "</thead><tbody>";

  foreach ($results as $item) {
    $name = $item->name;
    $state = variable_get($name, NULL);

    $display .= "<tr>";

    if (isset($state['saved_values'][PAGE_1]['publication']['title'])) {
      $title = check_plain($state['saved_values'][PAGE_1]['publication']['title']);
      $display .= "<td><a href='$base_url/tpps?accession={$state['accession']}' style='text-decoration: underline;'>$title</a></td>";
    }
    else {
      $display .= "<td></td>";
    }

    if (isset($state['saved_values'][PAGE_1]['primaryAuthor'])) {
      $author = check_plain($state['saved_values'][PAGE_1]['primaryAuthor']);
      $display .= "<td>$author</td>";
    }
    else {
      $display .= "<td></td>";
    }

    if (isset($state['saved_values'][PAGE_1]['organism']['number'])) {
      $species_number = check_plain($state['saved_values'][PAGE_1]['organism']['number']);
      $display .= "<td>$species_number</td>";
    }
    else {
      $display .= "<td></td>";
    }

    if (isset($state['saved_values'][PAGE_1]['publication']['abstract'])) {
      $abstract = check_plain($state['saved_values'][PAGE_1]['publication']['abstract']);
      if (strlen($abstract) > 103) {
        $abstract = check_plain(substr($abstract, 0, 100) . "...");
      }
      $display .= "<td>$abstract</td>";
    }
    else {
      $display .= "<td></td>";
    }

    if (isset($state['saved_values'][PAGE_1]['publication']['journal'])) {
      $journal = check_plain($state['saved_values'][PAGE_1]['publication']['journal']);
      $display .= "<td>$journal</td>";
    }
    else {
      $display .= "<td></td>";
    }

    if (isset($state['accession'])) {
      $display .= "<td style='text-align:center'><input type='submit' id='delete-{$state['accession']}' onclick='";
      $title = (isset($state['saved_values'][PAGE_1]['publication']['title'])) ? ' titled ' . $state['saved_values'][PAGE_1]['publication']['title'] : '';
      $display .= "if (confirm(\"Attention! \\n\\nAre you sure you want to delete your submission{$title}?\\nThis action cannot be undone, and all of your data will be lost!\")){window.location.href = \"$base_url/user/{$user->uid}/tpps?accession={$state['accession']}&action=DELETE\"}";
      $display .= "' name='op' class='form-button' value='Delete'></td>";
    }
    else {
      $display .= '<td></td>';
    }

    $display .= "</tr>";
  }

  if (!isset($name)) {
    $display = $style;
  }
  else {
    $display .= "</tbody></table></div><br>";
    $t1 = TRUE;
  }

  $final .= $display;
  $state_name = 'tpps_complete_' . $email;
  $name = NULL;

  $results = db_select("public.variable", "variable")
    ->fields('variable')
    ->condition('name', db_like($state_name) . '%', 'LIKE')
    ->execute();

  $display = "<label for='tpps_complete_table'>Complete Submissions</label><div class='tpps_profile_tab'><table id='tpps_complete_table' style='width:-webkit-fill-available' border='1'><thead>";
  $display .= "<tr><th>Title</th><th>Primary Author</th><th>Number of Species</th><th>Abstract</th><th>Journal</th><th>Status</th></tr>";
  $display .= "</thead><tbody>";

  foreach ($results as $item) {
    $name = $item->name;
    $state = variable_get($name, NULL);

    $display .= "<tr>";

    if (isset($state['saved_values'][PAGE_1]['publication']['title'])) {
      $title = check_plain($state['saved_values'][PAGE_1]['publication']['title']);
      $display .= "<td><a href='$base_url/completed-submission?id={$state['accession']}' style='text-decoration: underline;'>$title</a></td>";
    }
    else {
      $display .= "<td></td>";
    }

    if (isset($state['saved_values'][PAGE_1]['primaryAuthor'])) {
      $author = check_plain($state['saved_values'][PAGE_1]['primaryAuthor']);
      $display .= "<td>$author</td>";
    }
    else {
      $display .= "<td></td>";
    }

    if (isset($state['saved_values'][PAGE_1]['organism']['number'])) {
      $species_number = check_plain($state['saved_values'][PAGE_1]['organism']['number']);
      $display .= "<td>$species_number</td>";
    }
    else {
      $display .= "<td></td>";
    }

    if (isset($state['saved_values'][PAGE_1]['publication']['abstract'])) {
      $abstract = check_plain($state['saved_values'][PAGE_1]['publication']['abstract']);
      if (strlen($abstract) > 103) {
        $abstract = check_plain(substr($abstract, 0, 100) . "...");
      }
      $display .= "<td>$abstract</td>";
    }
    else {
      $display .= "<td></td>";
    }

    if (isset($state['saved_values'][PAGE_1]['publication']['journal'])) {
      $journal = check_plain($state['saved_values'][PAGE_1]['publication']['journal']);
      $display .= "<td>$journal</td>";
    }
    else {
      $display .= "<td></td>";
    }

    $display .= "<td>{$state['status']}</td></tr>";
  }

  if (!isset($name)) {
    $display = "";
  }
  else {
    $display .= "</tbody></table></div><br>";
    $t2 = TRUE;
  }

  if (!$t1 and !$t2) {
    $display = $style;
    $display .= "<h2>Could not find any Submissions</h2>";
    $display .= "<p>We could not find any incomplete TPPS Submissions or TPPS Submissions pending approval. If you would like to start a new TPPS Submission, you can start <a href=$base_url/tpps>here.</a></p>";
  }

  $final .= $display;

  return $final;
}
