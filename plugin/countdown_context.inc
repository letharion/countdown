<?php
/**
 * @file
 * Expose the countdown dates as context conditions.
 */

class countdown_context_condition extends context_condition {
  /**
   * List timers available as conditions.
   */
  public function condition_values() {
    $values = array();

    $query = db_select('countdowns', 'c');
    $countdowns = $query->fields('c')
      ->execute()
      ->fetchAll();

    foreach ($countdowns as $countdown) {
      $values['countdown_' . $countdown->cid] = t('Timer block targeting @time', array('@time' => date('Y-m-d H:i:s', $countdown->target_time)));
    }

    return $values;
  }

  /**
   * Display condition options.
   */
  public function options_form($context) {
    $defaults = $this->fetch_from_context($context, 'options');
    $options = array(
      '>=' =>  t('Current time is equal to or after countdown target time.'),
      '<' => t('Current time is before countdown target time.'),
    );
    $form['operation'] = array(
      '#title' => t('Operation'),
      '#type' => 'select',
      '#options' => $options,
      '#description' => t('The comparison to perform to determine if the countdown meets the condition.'),
      '#default_value' => isset($defaults['operation']) ? $defaults['operation'] : '',
      '#required' => TRUE,
    );
    return $form;
  }

  /**
   * Ceck the context condition.
   */
  public function execute() {
    foreach ($this->get_contexts() as $context) {
      $options = $this->fetch_from_context($context, 'options');
      $countdowns = $this->fetch_from_context($context, 'values');

      $cids = array();
      foreach ($countdowns as $countdown) {
        $cids[] = substr($countdown, 10);
      }

      $countdowns = db_select('countdowns', 'c')
        ->fields('c', array('target_time'))
        ->condition('cid', $cids, 'IN')
        ->execute()
        ->fetchAll();

      foreach ($countdowns as $countdown) {
        if (empty($countdown->target_time)) {
          continue;
        }

        $target_time = $countdown->target_time;
        $compare_time = time();

        switch ($options['operation']) {
          case '>=':
            if ($compare_time >= $target_time) {
              $this->condition_met($context);
            }
            break;

          case '<':
            if ($compare_time < $target_time) {
              $this->condition_met($context);
            }
            break;
        }
      }
    }
  }
}
