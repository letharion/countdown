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
      '>=' =>  t('Current time is equal or after countdown target time.'),
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
    $form['value'] = array(
      '#title' => t('Value'),
      '#type' => 'textfield',
      '#description' => t("The value the countdown should contain to meet the condition. This can either be an absolute date in ISO format (YYYY-MM-DDTHH:MM:SS) or a relative string like '12AM today'. Examples: 2011-12-31T00:00:00, now, now +1 day, 12AM today, Monday next week. <a href=\"@relative_format\">More examples of relative date formats in the PHP documentation</a>.", array('@relative_format' => 'http://www.php.net/manual/en/datetime.formats.relative.php')),
      '#default_value' => isset($defaults['value']) ? $defaults['value'] : '',
      '#process' => array('ctools_dependent_process'),
      '#dependency' => array(
        ':input[name="conditions[plugins][countdown_context_date_condition][options][operation]"]' => array(
          '<', '<=', '>', '>=', '=', '!=',
        ),
      ),
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
        $compare_time = strtotime($options['value']);

        switch ($options['operation']) {
          case '>=':
            if ($target_time >= $compare_time) {
              $this->condition_met($context);
            }
            break;

          case '<':
            if ($target_time < $compare_time) {
              $this->condition_met($context);
            }
            break;
        }
      }
    }
  }
}
