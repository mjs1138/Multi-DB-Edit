<?php
/**
 * @file
 * This is the Form API Tutorial from the handbook.
 *
 * It goes through several form examples of increasing complexity to demonstrate
 * Drupal 7 Form API.
 *
 * Links are provided inline for the related handbook pages.
 *
 * @see http://drupal.org/node/262422
 */
require_once(__DIR__ . '/../DBAccess.config');

function entryForm($form, &$form_state)
{
    $form['dbTable'] = array(
        '#type' => 'textfield',
        '#title' => t('DB Table'),
        '#required' => FALSE,
    );
    $form['dbField'] = array(
        '#type' => 'textfield',
        '#title' => t('DB Field'),
        '#required' => FALSE,
    );
    $form['targetValue'] = array(
        '#type' => 'textfield',
        '#title' => t('Target Value'),
        '#required' => FALSE,
    );
    $form['replaceValue'] = array(
        '#type' => 'textfield',
        '#title' => t('Relace Value'),
        '#required' => FALSE,
    );
    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Submit',
    );
    return $form;
}

/**
 * @param $form
 * @param $form_state
 */
function entryForm_submit(&$form, &$form_state)
{
    session_unset(); // Clear Session Array
    // Store the form-state in $_SESSION[]
    $_SESSION['data']['table'] = $form_state['values']['dbTable'];
    $_SESSION['data']['field'] = $form_state['values']['dbField'];
    $_SESSION['data']['targetValue'] = $form_state['values']['targetValue'];
    $_SESSION['data']['replaceValue'] = $form_state['values']['replaceValue'];

    // Form Returns to new URL
    $form_state['redirect'] = 'multiDBEdit/results';

    return;
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////


function outputForm($form, &$form_state)
{
    $targetTable = $_SESSION['data']['table'];
    $targetField = $_SESSION['data']['field'];
    $targetValue = $_SESSION['data']['targetValue'];
    $replaceValue = $_SESSION['data']['replaceValue'];
    $output = "";
    $tableRowS = array();

    $db_List_Header_Message = _buildDatabaseListz($targetTable, $targetField, $targetValue, $replaceValue);

    if (!empty($db_List_Header_Message['dbList'])) {  // There are DBs
        $a = 1;
        foreach ($db_List_Header_Message['dbList'] as $field) {
            if ($targetTable == null) {
                $tableHeader = array(t('Database'));
                $tableRowS[] = array($field);
            } elseif ($targetField == null) {
                $tableHeader = array(t('Database'), t('Table'));
                $tableRowS[] = array($field, $targetTable);
            } elseif ($targetValue == null) {
                $tableHeader = array(t('Database'), t('Table'), t('Field'));
                $tableRowS[] = array($field, $targetTable, $targetField);
            } elseif ($replaceValue == null) {
                $tableRowS[] = array($field, $targetTable, $targetField, $targetValue);
                $tableHeader = array(t('Database'), t('Table'), t('Field'), t('Field Value'));
            }
        }
    } else {
        // NO DBS FOUND????
    }

    if (!empty($db_List_Header_Message['dbList'])) {
        $output = array(
            '#theme' => 'table',
            '#header' => $tableHeader,
            '#rows' => $tableRowS,);
    } else {
        $output = array(
            '#theme' => 'table',
            '#header' => array(t('No Database(s) found w/ matching criteria.')),);
    }

    $message = array(
        '#markup' => $db_List_Header_Message['message'],
        '#prefix' => '<p><strong>',
        '#suffix' => '</strong></p>',
    );

    return array($output, $message); //!!!! output must be an array of array's
}

/**
 * @param $form
 * @param $form_state
 */
function outputForm_submit(&$form, &$form_state)
{
    $targetTable = $_SESSION['data']['table'];
    $targetField = $_SESSION['data']['field'];
    $targetValue = $_SESSION['data']['targetValue'];
    $replaceValue = $_SESSION['data']['replaceValue'];

    // Form Returns to new URL
    $form_state['redirect'] = 'multiDBEdit/results';
    return;
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////

function _buildDatabaseListz($targetTable = null, $targetField = null, $targetValue = null, $replaceValue = null)
{
    $dbList = array();
    $db_List_Message = array();
    $message = "";
    $nonConFlag = false;

    try {
        $dbh = new PDO(sprintf('mysql:host=%s', DB_HOST), DB_USER, DB_PASS); //PDO without selecting DB
    } catch (PDOException $e) {
        $message = 'Database Connection failed: ' . $e->getMessage();
    }

    $dbS = $dbh->query('SHOW DATABASES');
    $databaseS = $dbS->fetchAll(PDO::FETCH_ASSOC);

    $result = false;
    $tableHeader = "";
    foreach ($databaseS as $db) {
        $ndb = $dbh->query('USE ' . $db['Database']); // test if valid DB name (i.e. no '.' or '-')

        if ($ndb) { // test for valid DB to use
            if ($targetTable == null) { // Show all databases available
                $result = true;
                $tableHeader = array(t('Database'));
            } elseif ($targetField == null) { // just show databases w/ targetTable
                $resultSet = $dbh->query("SELECT * FROM `$targetTable`");
                if ($resultSet) {
                    $tableHeader = array(t('Database'), t('Table'));
                }
            } elseif ($targetValue == null) { // just show databases w/ targetTable & targetValue
                $resultSet = $dbh->query("SELECT `$targetField` FROM `$targetTable`");
                if ($resultSet) {
                    $tableHeader = array(t('Database'), t('Table'), t('Field'));
                }
            } elseif ($replaceValue == null) {
                $resultSet = $dbh->query("SELECT `$targetField` FROM `$targetTable` WHERE `$targetField` = '$targetValue'");
                if ($resultSet) {
                    $result = $resultSet->fetchAll(PDO::FETCH_ASSOC); // resultSet must be tested for not only successful execution but for having returned table rows as well
                    if (!empty($result)) {
                        $tableHeader = array(t('Database'), t('Table'), t('Field'), t('Field Value'));
                    }
                }
            }
            if ($tableHeader) { // test for valid DB to use
                $dbList[] = $db['Database'];
                $tableHeader = "";
            }
        } else { // non-cononical DB name
            $nonConFlag = true;
        }

    }

    if ($nonConFlag) {
        $message = "Databases w/ non-canonical names were found";
    }

    $db_List_Message['dbList'] = $dbList;
    $db_List_Message['header'] = $tableHeader;
    $db_List_Message['message'] = $message;
    return $db_List_Message;
}
