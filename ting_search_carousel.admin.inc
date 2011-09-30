<?php
/**
 * @file
 *
 * Admin related backend.
 */

/**
 * Admin page menu callback.
 *
 * @return
 *   Admin form
 */
function ting_search_carousel_admin_page() {
  drupal_add_js(drupal_get_path('module', 'ting_search_carousel'). '/js/ting_search_carousel.admin.js');
  drupal_add_css(drupal_get_path('module', 'ting_search_carousel'). '/css/ting_search_carousel.css');

  return drupal_get_form('ting_search_carousel_admin_form');
}

/**
 * Search queries admin form.
 *
 * @param $form
 *   Form
 * @param $form_state
 *   Form state
 * @return
 *   Form structure
 */
function ting_search_carousel_admin_form($form, &$form_state) {
  $searches_num = !empty($form_state['values']['num_searches']) ? $form_state['values']['num_searches'] : 0;

  $form['ting_search_carousel'] = array(
    '#type' => 'fieldset',
    '#title' => 'Searches',
  );

  // Get current saved queries
  $searches = variable_get('ting_carousel_search_queries', array());
  
  $i = 0;
  $c = 1;

  // Display saved queries
  if (isset($searches[0]['title'])) {
    $c = count($searches);
    for(; $i < $c; $i++) {
      $form['ting_search_carousel']['ting_searches']['search_' . $i] = ting_search_carousel_query_form($searches[$i]);
    }
  }

  // Display newly created fields
  for(; $i < $searches_num + $c; $i++) {
    $form['ting_search_carousel']['ting_searches']['search_' . $i] = ting_search_carousel_query_form();
  }

  // Keep track of fields count
  $form['ting_search_carousel']['num_searches'] = array(
    '#type' => 'hidden',
    '#value' => $searches_num += 1,
  );

  // Ajaxified button for new fields
  $form['ting_search_carousel']['new_search_carousel_button'] = array(
    '#type' => 'button',
    '#value' => 'Add another',
    '#ajax' => array(
      'callback' => 'ting_search_carousel_add_query',
      'wrapper' => 'ting-search-carousel-queries',
      'method' => 'replace',
      'effect' => 'fade'
    ),
  );

  // Cache length
  $form['ting_search_carousel']['ting_search_carousel_cache_duration'] = array(
    '#type' => 'select',
    '#title' => t('Cache duration'),
    '#description' => t('Determines how long search results in the carousel will be cached for.'),
    '#options' => array(
      3600 => t('1 hour'),
      10800 => t('3 hours'),
      43200 => t('12 hours'),
      86400 => t('1 day'),
      259200 => t('3 days'),
      604800 => t('7 days'),
      1209600 => t('2 weeks'),
      2592000 => t('1 month'),
    ),
    '#default_value' => variable_get('ting_search_carousel_cache_duration', 604800),
  );

  // Custom form submit handler
  $form['#prefix'] = '<div id="ting-search-carousel-queries">';
  $form['#suffix'] = '</div>';
  $form['#theme'] = array('ting_search_carousel_admin_form');
  $form['#submit'] = array('ting_search_carousel_search_submit');
  
  return system_settings_form($form);
}

/**
 * AJAX responder for displaying new fields.
 *
 * @param $form
 *   Form
 * @return
 *   Changed field to be updated
 */
function ting_search_carousel_add_query($form) {
  //$c = count(element_children($form['ting_search_carousel']['ting_searches']));
  
  return $form;//['ting_search_carousel'];//['ting_searches']['search_' . ($c - 1)];
}

/**
 * Theme the queries form, mainly adding a table drag-n-drop feature.
 *
 * @param $variables
 *   Theme related variables
 * @return
 *   Themed form output
 */
function theme_ting_search_carousel_admin_form($variables) {
  $form = $variables['form'];
  drupal_add_tabledrag('ting-search-carousel-admin-sort', 'order', 'sibling', 'sort');

  $header = array('Searches', '');
  $rows = array();
  foreach (element_children($form['ting_search_carousel']['ting_searches']) as $key) {
    // Add class to group weight fields for drag and drop.
    $form['ting_search_carousel']['ting_searches'][$key]['sort[]']['#attributes']['class'][] = 'sort';

    // Get sort element
    $sort = $form['ting_search_carousel']['ting_searches'][$key]['sort[]'];
    unset($form['ting_search_carousel']['ting_searches'][$key]['sort[]']);

    // Build rows
    $rows[] = array(
      'data' => array(
        drupal_render($form['ting_search_carousel']['ting_searches'][$key]),
        drupal_render($sort)
      ),
      'class' => array('draggable')
    );
  }

  $table_vars = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'ting-search-carousel-admin-sort'),
    'caption' => '',
    'colgroups' => array(),
    'sticky' => FALSE,
    'empty' => 'EMPTY'
  );
  $output = theme_table($table_vars);

  // Render remaining elements
  $output .= drupal_render_children($form);

  return $output;
}

/**
 * Form part, describing query fields.
 *
 * @param $item
 *   Values for existing query
 * @return
 *   Fields structure
 */
function ting_search_carousel_query_form($item = array()) {
  $form = array();

  $form['sort[]'] = array(
    '#type' => 'weight',
    '#delta' => 5,
    '#default_value' => 0,
    '#value' => 0,
    '#attributes' => array(
      'class' => array(
        'sort-hidden'
       )
    ),
  );

  $form['title[]'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#description' => t('The title of the search is used for the tab in the carousel. Keep it short.'),
    '#default_value' => isset($item['title']) ? $item['title'] : '',
    '#prefix' => '<div class="search-carousel-query">',
  );

  $form['subtitle[]'] = array(
    '#type' => 'textfield',
    '#title' => t('subtitle'),
    '#description' => t('The subtitle displayed next to the search results'),
    '#default_value' => isset($item['subtitle']) ? $item['subtitle'] : '',
  );

  $form['query[]'] = array(
    '#type' => 'textfield',
    '#title' => t('Query'),
    '#maxlength' => 2048,
    '#description' => t('The query to display search results for'),
    '#default_value' => isset($item['query']) ? $item['query'] : '',
  );
  
  $form['remove[]'] = array(
    '#type' => 'button',
    '#submit' => FALSE,
    '#value' => t('Remove'),
    '#attributes' => array(
      'class' => array(
        'remove'
      )
    ),
    '#suffix' => '</div><div class="clear"></div>',
  );

  return $form;
}

/**
 * Submit handler for admin backend queries form.
 *
 * @param $form
 *   Form
 * @param $form_state
 *   Form state
 */
function ting_search_carousel_search_submit($form, &$form_state) {
  $c = count($form_state['input']['title']);
  $searches = array();

  // Make an array for saving, ignoring queries w/o title
  for($i = 0; $i < $c; $i++) {
    if (!empty($form_state['input']['title'][$i])) {
      $searches[] = array(
        'title' => $form_state['input']['title'][$i],
        'subtitle' => $form_state['input']['subtitle'][$i],
        'query' => $form_state['input']['query'][$i]
      );
    }
  }

  // Save the queries as a persistent variable
  variable_set('ting_carousel_search_queries', $searches);
}
