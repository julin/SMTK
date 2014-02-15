#include "smtk/model/BridgeBase.h"


namespace smtk {
  namespace model {

/**\brief Transcribe an entity from a foreign modeler into SMTK Storage.
  *
  * On input, the \a entity will not be valid but if transcription is
  * successful, the \a requested records in the \a entity's Storage will
  * be valid. If \a requested includes BRIDGE_ENTITY_TYPE, then
  * \a entity.isValid() should return true after this call.
  *
  * Only honor requests for entity IDs listed as dangling unless
  * \a onlyDangling is false (default is true).
  * This prevents expensive requests by Storage instances over many Bridges.
  *
  * The return value is 0 upon failure and non-zero upon success.
  * Failure occurs when any \a requested bits of information that
  * are in BridgeBasee::allSupportedInformation() are not transcribed,
  * or when \a requested is 0.
  */
int BridgeBase::transcribe(
  const Cursor& entity, BridgedInfoBits requested, bool onlyDangling)
{
  int retval = 0;
  if (requested)
    {
    // Check that the entity IDs is dangling or we are forced to continue.
    DanglingEntities::iterator it = this->m_dangling.find(entity);
    if (onlyDangling && it == this->m_dangling.end())
      { // The bridge has not been told that this UUID exists.
      return retval;;
      }
    // Ask the subclass to transcribe information.
    BridgedInfoBits actual = this->transcribeInternal(entity, requested);
    // Decide which bits of the request can possibly be honored...
    BridgedInfoBits honorable = requested & this->allSupportedInformation();
    // ... and verify that all of those have been satisfied.
    retval = (honorable & actual) == honorable;
    // If transcription is complete, then remove the UUID from the dangling
    // entity set:
    if (
      ((actual & this->allSupportedInformation()) == this->allSupportedInformation()) &&
      (it != this->m_dangling.end()))
        this->m_dangling.erase(it);
    }
  return retval;
}

/**\brief Return a bit vector describing what types of information can be transcribed.
  *
  * This is used to determine when an entity has been fully transcribed into storage
  * and is no longer "dangling."
  */
BridgedInfoBits BridgeBase::allSupportedInformation() const
{
  return BRIDGE_EVERYTHING;
}

/**\brief Mark an entity, \a ent, as partially transcribed.
  *
  * Subclasses should call this method when a UUID has been assigned
  * to a model entity but ent.storage() has not yet been populated with
  * all of the information about the entity. The information which *is*
  * \a present in ent.storage() should be passed but will default to
  * zero (i.e., the UUID exists in some other entity's relations but
  * has no records in storage itself).
  *
  * The entity is added to the list of dangling entities and will be
  * removed from the list when a call to \a transcribeInternal indicates
  * that BridgeBase::allSupportedInformation() is now present in storage.
  */
void BridgeBase::declareDanglingEntity(const Cursor& ent, BridgedInfoBits present)
{
  if ((present & this->allSupportedInformation()) < this->allSupportedInformation())
    this->m_dangling[ent] = present;
}

/**\brief Transcribe information requested by \a flags into \a entity from foreign modeler.
  *
  * Subclasses must override this method.
  * This method should return a non-zero value upon success.
  * Upon success, \a flags should be modified to represent the
  * actual information transcribed (as opposed to what was requested).
  * This should always be at least the information requested but may
  * include more information.
  */
BridgedInfoBits BridgeBase::transcribeInternal(const Cursor& entity, BridgedInfoBits flags)
{
  (void)entity;
  (void)flags;
  // Fail to transcribe anything:
  return 0;
}

  } // namespace model
} // namespace smtk