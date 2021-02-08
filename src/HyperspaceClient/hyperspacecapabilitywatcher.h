/*
 *
 */

#ifndef HYPERSPACE_CAPABILITYWATCHER_H
#define HYPERSPACE_CAPABILITYWATCHER_H

#include <HemeraCore/AsyncInitObject>

namespace Hyperspace {

/**
 * @class CapabilityWatcher
 * @ingroup Client
 * @headerfile HyperspaceClient/hyperspacecapabilitywatcher.h <HyperspaceClient/CapabilityWatcher>
 *
 * @brief Watches over a list of capabilities throughout all Hyperspace accessible devices.
 *
 * CapabilityWatcher exposes Hyperspace's discovery features as a stateful object, monitoring capabilities
 * the user is interested in. To minimize data transfer and processing, the user MUST specify a set of
 * capabilities he wants to monitor.
 *
 * When using CapabilityWatcher, the user needs to specify both the set of Capabilities to watch for
 * and a set of WatchedEvents. WatchedEvents are specific to a CapabilityWatcher: if you need to monitor
 * different capabilities for different events, create different CapabilityWatchers.
 *
 * @note
 * Into an Hemera Application, any number of CapabilityWatchers can be created without any relevant
 * impact on performances, due to the fact the processing and transfer logic is centralized among
 * all the discovery classes.
 *
 * @note
 * Semantics in this class are similar to @ref QDBusServiceWatcher 's ones.
 *
 * @sa QDBusServiceWatcher
 */
class CapabilityWatcher : public Hemera::AsyncInitObject
{
    Q_OBJECT

public:
    /**
     * Defines the status of a specific Capability. Several discovery services might expose the same
     * capability, hence this represents a grouped presence of a specific capability over a variety
     * of services.
     *
     * To retrieve the list of URLs the capability can be accessed from, you will need to introspect it.
     */
    enum class Status : quint8 {
        /// The capability is in an unknown state. It probably has been never advertised.
        Unknown = 1 << 0,
        /// The capability has been announced and is present on one or more networks.
        Announced = 1 << 1,
        /// The capability has expired or been purged from every known network.
        Expired = 1 << 2,
        /// The capability has been purged from every known network.
        Purged = 1 << 3
    };

    /**
     * Specifies events CapabilityWatcher can watch for. Use the WatchedEvents flags to specify
     * the behavior of a CapabilityWatcher instance.
     */
    enum class WatchedEvent : quint8 {
        /// Watches for no event.
        NoEvent = 1 << 0,
        /// Watches for "Announced" event.
        Announced = 1 << 1,
        /// Watches for "Expired" event.
        Expired = 1 << 2,
        /// Watches for "Purged" event.
        Purged = 1 << 3,
        /// Watches for introspection changes, notifying when URLs referring to this capability have changed.
        Introspection = 1 << 4,
        /// Watches introspection data, and processes the actual introspection as well. Increases significantly data
        /// exchanges over the pipes.
        IntrospectionData = 1 << 5,

        // Convenience pre-made flags
        /// Watches for all presence events.
        PresenceEvents = Announced | Expired | Purged | Introspection,
        /// Watches for every event, and downloads services URL data. Uses a lot of bandwidth.
        All = Announced | Expired | Purged | IntrospectionData,
    };
    Q_DECLARE_FLAGS(WatchedEvents, WatchedEvent);

    /**
     * Constructs a CapabilityWatcher object, and starts monitoring for @p capabilities right after its initialization.
     *
     * @p capabilities A list of capabilities to watch for
     * @p watchedEvents The events this @ref CapabilityWatcher should watch for for its @p capabilities
     * @p parent A parent for QObject's parenting system.
     */
    explicit CapabilityWatcher(const QList<QByteArray> &capabilities, WatchedEvents watchedEvents = WatchedEvent::PresenceEvents, QObject *parent = 0);
    explicit CapabilityWatcher(const QByteArray &capability, WatchedEvents watchedEvents = WatchedEvent::PresenceEvents, QObject *parent = 0);

    /**
     * Destroys the object, and stops watching for the capabilities it was assigned to.
     */
    virtual ~CapabilityWatcher();

    /**
     * Adds more capabilities to the list of watched capabilities.
     *
     * @note This method can be called only after the object has been initialized.
     *
     * @p capabilities The list of additional capabilities to watch for
     */
    void addCapabilities(const QList<QByteArray> &capabilities);
    /**
     * Removes capabilities from the list of watched capabilities.
     *
     * @note This method can be called only after the object has been initialized.
     *
     * @p capabilities The list of capabilities not to watch for anymore.
     */
    void removeCapabilities(const QList<QByteArray> &capabilities);

    /**
     * @returns the @ref WatchedEvents of this @ref CapabilityWatcher
     */
    WatchedEvents watchedEvents() const;

protected:
    virtual void initImpl() Q_DECL_OVERRIDE Q_DECL_FINAL;

Q_SIGNALS:
    /**
     * Emitted upon the status changed of a monitored capability.
     *
     * This signal will be emitted only for those statuses enabled in @ref watchedEvents and for this @ref CapabilityWatcher 's
     * assigned capabilities. Any other change in Hyperspace will not be notified.
     *
     * @p capability The changed capability
     * @p status The new status of @p capability
     */
    void capabilityStatusChanged(const QByteArray &capability,
                                 Hyperspace::CapabilityWatcher::Status status);
    /**
     * Emitted upon the introspection change of a monitored capability.
     *
     * This signal will be emitted only if @ref CapabilityWatcher 's @ref watchedEvents include @ref Introspection.
     *
     * Whenever @ref Introspection changes, the capability changed its list of accessible URLs. If you are interested
     * in those changes, you will need to introspect the capability or, in case you are always interested in the URL
     * list, you can instead watch for @ref IntrospectionData and listen to @ref capabilityIntrospectionDataChanged.
     *
     * @p capability The changed capability
     */
    void capabilityIntrospectionChanged(const QByteArray &capability);
    /**
     * Emitted upon the introspection change of a monitored capability.
     *
     * This signal will be emitted only if @ref CapabilityWatcher 's @ref watchedEvents include @ref IntrospectionData.
     *
     * Whenever @ref Introspection changes, the capability changed its list of accessible URLs. With this signal,
     * the capability is introspected and the URL list delivered. Please note you should use this signal only if you
     * are always interested in the capability's URLs, as it increases data exchange on Hyperspace pipes significantly.
     *
     * @p capability The changed capability
     */
    void capabilityIntrospectionDataChanged(const QByteArray &capability, const QList<QByteArray> &servicesUrl);

private:
    class Private;
    Private * const d;

    friend class GlobalDiscoveryWatcher;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Hyperspace::CapabilityWatcher::WatchedEvents)

#endif // HYPERSPACE_CAPABILITYWATCHER_H
