<?php

/**
 * @file
 * Define the TPPS landing page.
 *
 * This page can be accessed even by anonymous users. If the user is logged in,
 * they are able to select an existing TPPS Submission, or create a new one.
 */

/**
 * Creates the landing page and its form.
 *
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 *
 * @global string $base_url
 *   The base url of the site.
 * @global stdClass $user
 *   The user trying to access the form.
 *
 * @return array
 *   The form for the TPPS landing page.
 */
function tpps_front_create_form(array &$form, array $form_state) {
  global $base_url;
  global $user;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (Submission::isCurationForm($form_state)) {
    if (user_is_logged_in()) {
      // Logged in.
      $options = [
        'new' => 'Create new TPPSC Submission',
      ];
      // Incomplete own studies for logged-in user.
      $list = SubmissionCache::getAccessionList([
        [['status'], Submission::STATUS_INCOMPLETE],
        [['submitter', 'uid'], $user->uid],
      ]);
      if ($list) {
        $options = $options + [' YOUR / INCOMPLETE ' => $list];
      }
      if (variable_get('tpps_front_show_pending_status_mine', FALSE)) {
        $list = SubmissionCache::getAccessionList([
          [['status'], Submission::STATUS_PENDING_APPROVAL],
          [['submitter', 'uid'], $user->uid],
        ]);
        if ($list) {
          $options = $options + [' YOUR / PENDING APPROVAL' => $list];
        }
      }
      if (variable_get('tpps_front_show_approved_status_mine', FALSE)) {
        $list = SubmissionCache::getAccessionList([
          [['status'], Submission::STATUS_APPROVED],
          [['submitter', 'uid'], $user->uid],
        ]);
        if ($list) {
          $options = $options + [' YOUR / APPROVED ' => $list];
        }
      }
      if (variable_get('tpps_front_show_others_studies', TRUE)) {
        $list = SubmissionCache::getAccessionList([
          [['status'], Submission::STATUS_INCOMPLETE],
          [['submitter', 'uid'], $user->uid, '<>'],
        ]);
        if ($list) {
          $options = $options + [' OTHERS / INCOMPLETE ' => $list];
        }
      }
      if (variable_get('tpps_front_show_pending_status_others', FALSE)) {
        $list = SubmissionCache::getAccessionList([
          [['status'], Submission::STATUS_PENDING_APPROVAL],
          [['submitter', 'uid'], $user->uid, '<>'],
        ]);
        if ($list) {
          $options = $options + [' OTHERS / PENDING APPROVAL' => $list];
        }
      }
      if (count($options) > 1) {
        $form['accession'] = [
          '#type' => 'select',
          '#title' => t('Would you like to load an old TPPSC submission, '
            . 'or create a new one?'),
          '#options' => $options,
          '#default_value' => $form_state['saved_values']['frontpage']['accession'] ?? 'new',
        ];
        TppsForm::autofocus($form, ['accession']);
        $form['use_old_tgdr'] = [
          '#type' => 'checkbox',
          '#title' => t('I would like to use an existing TGDR number'),
          '#default_value' => FALSE,
        ];
        $form['old_tgdr'] = [
          '#type' => 'select',
          '#title' => t('Existing TGDR number'),
          '#options' => tpps_submission_get_tgdr_number_list($is_optional = TRUE),
          '#description' => '<div class="error">'
            . t('WARNING: Using this TGDR number will clear all data '
            . 'associated with this study!')
            . '</div>',
          '#states' => [
            'visible' => [
              ':input[name="use_old_tgdr"]' => ['checked' => TRUE],
            ],
          ],
        ];
      }
    }
    $form['Next'] = ['#type' => 'submit', '#value' => t('Continue to TPPSC')];

    $prefix_text = "<div>Welcome to TPPSC!<br><br>"
      . "If you would like to submit your data, you can click the button "
      . "'Continue to TPPSC' below!<br><br></div>";

    if (isset($form['accession'])) {
      $form['accession']['#prefix'] = $prefix_text;
    }
    else {
      $form['Next']['#prefix'] = $prefix_text;
    }

  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // TPPS Form.
  else {
    // @TODO [VS] Move HTML code to template or theme function.
    /*
    $prefix_text = '<div><figure style="text-align:center;"><img '
        . 'style="max-height:100%;max-width:100%;" src="/' . TPPS_IMAGES_PATH
        . 'TPPS-1_1118px.jpg"></figure>'
      . '<div id="landing-buttons">'
      . '<a href="https://cartograplant-tpps.readthedocs.io/en/latest/" target="blank" '
        . 'class="landing-button"><button type="button" class="btn '
        . 'btn-secondary">TPPS Documentation</button></a>'
        . '<a href="' . $base_url . '/tpps/details" target="blank" '
          . 'class="landing-button"><button type="button" '
          . 'class="btn btn-secondary">TPPS Studies</button></a>';

    if (module_exists('cartogratree')) {
      $prefix_text .= "<a href=\"$base_url/ct\" target=\"blank\" class=\"landing-button\"><button type=\"button\" class=\"btn btn-secondary\">CartograPlant</button></a>";
    }

    $prefix_text .= "</div></div>";
    */
    $prefix_text = '<style>';
    $prefix_text .= 'form { padding: 0px; }';
    $prefix_text .= '</style>';
    $prefix_text .= '<div>';
    $prefix_text .= '<div class="ct_flowchart" style="text-align: center;">';
    $prefix_text .= ' <div style="font-size: 24px; font-weight: 500; letter-spacing: 0px; line-height: 32px; margin-top: 20px; margin-bottom:20px;">SUBMIT YOUR STUDY TO CARTOGRAPLANT</div>';
    $prefix_text .= ' <hr style="border: solid 4px #e7e7e7; border-bottom: solid 0px #000000 !important;">';
    $prefix_text .= ' <p>';
    $prefix_text .= 'The Tripal Plant PopGen Submit (TPPS) pipeline is the data submission portal of CartograPlant by which researchers deposit georeferenced population genomics studies. A submitter provides four categories of information in a guided, multi-page form: publication metadata (authors, title, journal, DOI, or an unpublished study), study design (species, number of accessions, tissue sampled, sequencing or genotyping platform), per-accession location data (latitude and longitude, or site identifiers resolved to coordinates), and the genotype, phenotype, or environmental measurements themselves. Genotype data are accepted as VCF or as SNP, SSR, or other marker matrices, and phenotype data are described with controlled vocabularies (Plant Trait Ontology, Plant Ontology, and the appropriate unit ontologies) so that traits are comparable across studies.
Once submitted, the pipeline validates file formats and cross-checks accession identifiers across the location, genotype, and phenotype files, then stages the study for curator review. Once approved, the data are loaded into the Chado schema underlying the Tripal site, each study receives a stable TGDR accession and optionally, a DOI from Zenodo, and the accessions become visible on the CartograPlant map, where they can be filtered by species, trait, marker type, or environmental layer and combined with the site’s population genomics and environmental association analyses.
The final package is a FAIR-compliant and citable record that links the raw data to its publication and to the georeferenced samples.';
    $prefix_text .= ' </p>';
    $prefix_text .= '<img style="border: 2px solid #EFEFEF; max-height:100%;max-width:100%;" src="/' . TPPS_IMAGES_PATH . 'TPPS_front_ss.png">';
    $prefix_text .= ' <hr style="border: solid 4px #e7e7e7; border-bottom: solid 0px #000000 !important;">';
    $prefix_text .= '
    <table style="margin-left: auto; margin-right: auto; width: 50%; border: solid 0px #e7e7e7; border-bottom: solid 0px #e7e7e7;">
      <tbody>
      <tr>
        <td width="80%" style="padding-top: 20px;padding-bottom: 30px;padding-left: 12%;padding-right: 12%;border-top: 0px solid #000000;">
            <center>
            <table width="50%">
              <tbody>
              <tr>
                  <td style="width: 64px; padding: 10px;">
                    <img style="display: inline-block; width: 64px;" src="https://treegenesdb.org/sites/all/modules/custom_modules/cartogratree_landing/images/ct_icon_blog.png">
                  </td>
                  <td style="width: 40%; text-align: left; font-size: 18px;">
                    <a href="https://treegenesdb.org/tpps/details">TPPS Studies</a>
                  </td>
                  <td style="font-size: 10px; text-align: right; padding-right: 10px;">
                    Study details including accession numbers, species, and traits
                  </td>            
              </tr>
              
              <!--
              <tr>
                  <td colspan="3"><hr style="border: solid 2px #e7e7e7"></td>
              </tr>
              -->
            </tbody></table>
            <table width="50%">
              <tbody><tr>
                  <td style="width: 64px; padding: 10px;">
                    <img style="display: inline-block; width: 64px;" src="https://treegenesdb.org/sites/all/modules/custom_modules/cartogratree_landing/images/ct_icon_userguide.png">
                  </td>
                  <td style="width: 40%; text-align: left; font-size: 18px;">
                    <a href="https://cartograplant-tpps.readthedocs.io/en/latest/user.html">User Guide</a>
                  </td>
                  <td style="font-size: 10px; text-align: right; padding-right: 10px;">
                    Detailed documentation,<br> tutorials and resources
                  </td>            
              </tr>
              <!--
              <tr>
                  <td colspan="3"></td>
              </tr>
              -->
            </tbody></table>
            <table width="50%">
              <tbody>
              
              <tr>
                  <td style="width: 32px; padding: 10px; padding-left:15px; padding-right:15px; border-top: solid 0px #e7e7e7 !important;">
                    <img style="display: inline-block; width: 52px;" src="https://treegenesdb.org/sites/all/modules/custom_modules/cartogratree_landing/images/ct_icon_gettingstarted.png">
                  </td>
                  <td colspan="2">
                    <div style="display: inline-block; font-size: 20px;"><a href="https://treegenesdb.org/cartograplant">Browse Cartograplant</a></div>
                  </td>
              </tr>
            
              <!-- 
              <tr>
                  <td colspan="2"><hr style="border: solid 2px #e7e7e7"></td>
              </tr>
              -->
            </tbody>
            </table>           
            </center>
        </td>
      </tr>
      </tbody></table>
    ';

    // $prefix_text .= '<div id="landing-buttons">';
    // $prefix_text .= ' <a href="https://cartograplant-tpps.readthedocs.io/en/latest/" target="blank" class="landing-button">';
    // $prefix_text .= '   <button type="button" class="btn btn-secondary">TPPS Documentation</button>';
    // $prefix_text .= ' </a>';
    // $prefix_text .= ' <a href="' . $base_url . '/tpps/details" target="blank" class="landing-button">';
    // $prefix_text .= '   <button type="button" class="btn btn-secondary">TPPS Studies</button>';
    // $prefix_text .= ' </a>';
    // $prefix_text .= '</div>';


    $prefix_text .= ' </div>';
    $prefix_text .= '</div>';
    $prefix_text .= ' <hr style="border: solid 4px #e7e7e7; border-bottom: solid 0px #000000 !important;">';

    if (user_is_anonymous()) {
      // @todo Use css classes and theme-functions.
      $prefix_text .= '<div style="text-align: center">'
        . t("To begin submitting your data, please ensure that you're "
        . "logged in to access upload features on this page.")
        . '</div>';
      // @todo Use l() instead of <a> tags.
      $prefix_text .= '<div style="text-align: center">' .
        t("If you do not have an account <a style='color: #e2b448;' "
        . "href='/user/register'>register one here</a> or <a style='color: "
        . "#e2b448' href='/user/login'>click here to login</a>")
        . "</div>";
    }

    $form['description'] = ['#markup' => $prefix_text];
    // @TODO Check what anonymous users will see.
    if (user_is_logged_in()) {
      $options_arr = ['new' => 'Create new TPPS Submission']
        + SubmissionCache::getAccessionList([
          [['status'], Submission::STATUS_INCOMPLETE, '='],
          [['submitter', 'uid'], $user->uid, '='],
        ]);
      if (count($options_arr) > 1) {
        $form['accession'] = [
          '#type' => 'select',
          '#title' => t('Would you like to load an old TPPS submission, '
            . 'or create a new one?'),
          '#options' => $options_arr,
          '#default_value' =>
          $form_state['saved_values']['frontpage']['accession'] ?? 'new',
        ];
      }
      if (tpps_access('administer tpps module')) {
        $form['custom_accession_check'] = [
          '#type' => 'checkbox',
          '#title' => t('I would like to use a custom accession number'),
          '#description' => t('Specify a custom accession number. '
            . 'This feature is available only to users with administrative '
            . 'access, and is generally not required or recommended.'
          ),
        ];
        // This field allows to specify any accession number except existing
        // numbers. This is hard to guess and there is no autocompletion.
        $form['custom_accession'] = [
          '#type' => 'textfield',
          '#title' => t('Custom Accession number'),
          '#states' => [
            'visible' => [
              ':input[name="custom_accession_check"]' => ['checked' => TRUE],
            ],
          ],
          '#description' => t('Use this field to specify a custom accession '
            . 'number. Must be of the format TGDR###'
          ),
        ];
      }
      $form['Next'] = [
        '#prefix' => '<div style="text-align: center; margin-top: 20px;">',
        '#type' => 'submit',
        '#value' => t('Submit Data'),
        '#suffix' => '</div>',
        '#attributes' => array(
          'style' => 'margin-left:auto; margin-right: auto; padding: 20px;',
        ),
      ];
    }
  }
  tpps_add_css_js('main', $form);
  return $form;
}
