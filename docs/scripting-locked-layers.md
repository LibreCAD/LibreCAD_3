# Locked layers in scripts

Objects on a locked layer can be read and used for snapping, and new objects
can be added to that layer. Changes to existing objects are refused by default.

`EntityBuilder` rejects an entire operation if it would update or remove an
existing object on a locked layer. After `execute()`, call
`blockedByLockedLayer()` to check whether this happened. To deliberately edit
locked objects, call `setAllowLockedLayerEdits(true)` on that operation before
executing it. A refused operation retains its pending edit, so it can be
retried after unlocking or enabling the override. Undo and redo apply normally
to an allowed operation.

A compound `Builder` preflights direct edits and built-in entity-operation
stacks, including nested groups, against the lock state at the start of the
group. It also detects a supplied existing entity whose layer is locked by an
earlier `ReplaceLayer` in the group, including chained replacements; this case
is conservatively refused even for read-only or copy operations. It reports
refusal through its own `blockedByLockedLayer()` method.
Copying a supported built-in locked entity and filtering it out of an edit
remain allowed. An unknown operation or an unqualified custom copy with a
locked input is refused unless explicitly overridden. To unlock and edit in
one group, explicitly override the entity operation. Custom operations that
obtain locked objects outside their supplied inputs, entities created earlier
in the group, and other layer-state changes are not yet guaranteed atomic;
separate those operations when a locked entity may be involved. After a forced
edit using a stale entity snapshot, verify its layer metadata before saving.
Cyclic and null operation groups are rejected,
and every document operation requires a non-null document.

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
