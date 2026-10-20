# State List (`state_list`) — Drupal 7 Module

`state_list` — a module for Drupal 7 that provides a performant list manager (`StateList`) for persisting lists (scalar and associative key-value) between HTTP requests.

The module completely **eliminates the use of Drupal system variables (`variable_set()`)** for storing lists, removing constant database locks and cache resets.

---

## The Problem with `variable_set()` in Drupal 7

Using `variable_set()` to store frequently updated lists entails critical performance problems:

1. **DB write on every operation**: Every call to `variable_set()` executes a separate `MERGE` query into the `{variable}` table.
2. **Bootstrap cache reset**: On every call to `variable_set()`, Drupal invokes `cache_clear_all('variables', 'cache_bootstrap')`. If lists are updated in a loop (e.g., during an import or node processing), the bootstrap variables cache is invalidated hundreds of times per request.
3. **RAM bloat**: During page initialization (`bootstrap`), Drupal 7 loads **the entire `{variable}` table** into the global `$conf` array. Any large lists consume RAM on every page of the site, even where they are not needed.

---

## Architecture and Solution of `state_list`

1. **Buffering in static memory**: All read and modify operations (`add()`, `remove()`, `saveList()`, `clearList()`, `deleteList()`) during request execution happen in RAM (PHP static array).
2. **Single flush at end of request (`shutdown`)**: Modified and deleted lists are written to storage exactly **once** at the end of the PHP script via `drupal_register_shutdown_function()`.
3. **Dedicated `{state_list}` table**: Lists are stored in their own DB table (or in the `{cache_state_list}` cache bin). Data is loaded lazily only when accessed and **never resets `cache_bootstrap`**.
4. **Convenient settings page**: Storage backend configuration and a summary table of active lists are available in the Drupal control panel.

---

## Module Structure

```text
state_list/
├── state_list.info         # Module metadata and autoloading of src/StateList.class.inc
├── state_list.install      # Schema of {state_list} and {cache_state_list} tables, migration
├── state_list.module       # Storage initialization (hook_boot) and menu (hook_menu)
├── state_list.admin.inc    # Settings page and list overview (admin/config/system/state-list)
├── src/
│   └── StateList.class.inc # Main StateList class with buffering and backends
└── README.md               # Module documentation
```

---

## Installation

1. Unpack the `state_list.zip` archive into the modules folder (e.g., `sites/all/modules/custom/state_list`).
2. Enable the module via Drush or the administrator interface:
   ```bash
   drush en state_list -y
   ```
3. During installation, the `state_list_install()` hook will:
   * Create the `{state_list}` table and the `{cache_state_list}` cache table.

---

## Module Settings Page

Settings are available at:
**Configuration → System → State List settings** (`admin/config/system/state-list`).

The page provides:
* **Storage Backend selection**:
  * **Dedicated database table (`{state_list}`)** *(default, recommended)*: reliable persistent storage in its own DB table; data is preserved when clearing the cache with `drush cc all`.
  * **Drupal Cache API (`{cache_state_list}`)**: storage via the Cache API with the option to offload to Memcache or Redis.
* **Cache bin name**: when Cache API is selected, you can specify the cache table name (default `cache_state_list`).
* **Active lists overview**: a summary table of all stored lists with name, item count, list type (scalar or associative), and example values.

Settings can also be overridden via `settings.php`:
```php
$conf['state_list_storage_backend'] = 'table'; // or 'cache'
$conf['state_list_cache_bin'] = 'cache_state_list';
```

---

## API Usage Examples

The `StateList` class is registered in `state_list.info` via `files[] = src/StateList.class.inc` and is loaded automatically by the Drupal 7 core.

### 1. Adding items (`StateList::add`)

```php
// Add a unique value to a scalar list (no duplicates):
StateList::add('processed_nids', 101);
StateList::add('processed_nids', 102);
StateList::add('processed_nids', 101); // Ignored, since 101 is already in the list

// Add / update an item by key in an associative list:
StateList::add('user_tokens', 'token_abc123', 'uid_45');
StateList::add('user_tokens', 'token_xyz789', 'uid_99');
```

### 2. Retrieving and searching items (`getList`, `isExists`, `search`)

```php
// Get the entire list (returns array):
$nids = StateList::getList('processed_nids');
// Result: [0 => 101, 1 => 102]

// Check if a value exists in a scalar list:
if (StateList::isExists('processed_nids', 102)) {
  // Value 102 found
}

// Check if a key exists in an associative list:
if (StateList::isExists('user_tokens', NULL, 'uid_45')) {
  // Get value by key (returns FALSE if key not found):
  $token = StateList::search('user_tokens', NULL, 'uid_45');
}

// Search for a key/index by value (returns the index or FALSE):
$index = StateList::search('processed_nids', 102);
```

### 3. Removal and clearing (`remove`, `saveList`, `clearList`, `deleteList`)

```php
// Remove a value from a scalar list:
StateList::remove('processed_nids', 101);

// Remove an item by key from an associative list:
StateList::remove('user_tokens', NULL, 'uid_45');

// Completely replace list contents with a new array:
StateList::saveList('processed_nids', [201, 202, 203]);

// Clear the list (leaves an empty array []):
StateList::clearList('processed_nids');

// Completely delete the list from the database:
StateList::deleteList('processed_nids');

// Get all lists as an associative array:
$all = StateList::getAllLists();
```

### 4. Forced flush of changes to the DB (`flush`)

All changes are written to storage automatically at the end of the request via `drupal_register_shutdown_function()`.

If the code runs in a long batch process (Batch API, Drush command, Queue API) and changes need to be written to the DB immediately mid-execution:

```php
StateList::flush();
```
