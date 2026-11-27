# Locked layers in scripts

Objects on a locked layer can be read and used for snapping, and new objects
can be added to that layer. Changes to existing objects are refused by default.

`EntityBuilder` rejects an entire operation if it would update or remove an
existing object on a locked layer. After `execute()`, call
`blockedByLockedLayer()` to check whether this happened. To deliberately edit
locked objects, call `setAllowLockedLayerEdits(true)` on that operation before
executing it. Undo and redo apply normally to an allowed operation.

The script-facing document methods `insertEntity` and `removeEntity` also
respect the lock. They keep their existing no-result interface and log a
refusal. `tryInsertEntity` and `tryRemoveEntity` return a boolean instead;
`forceInsertEntity` and `forceRemoveEntity` explicitly bypass the layer lock.
Adding a new entity to a locked layer is allowed by either insert method.

The low-level C++ document and storage-manager methods remain available for
import, undo, and other trusted callers. The low-level storage-manager scripting
API also bypasses this policy; scripts that use it directly are responsible for
preserving locked objects.

Lua can construct a shared storage manager and document with
`lc.storage.StorageManagerImpl.create()` and
`lc.storage.DocumentImpl.create(manager)`. A document requires a non-null
storage manager.
