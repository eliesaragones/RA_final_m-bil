#ifndef MWC_TAXIBOT_TAXI_CORE_HPP
#define MWC_TAXIBOT_TAXI_CORE_HPP

#include <map>
#include <stdexcept>
#include <string>

namespace mwc_taxibot
{

enum class TaxiState
{
    IDLE,
    GOING_TO_PICKUP,
    WAITING_PASSENGER,
    GOING_TO_DESTINATION,
    CONFIRMING_ZONE,
    OBSTACLE_DETECTED,
    RETURNING_TO_BASE,
    ERROR
};

struct Zone
{
    std::string name;
    double x;
    double y;
    double yaw;
    std::string visual_label;
};

struct TaxiRequestResult
{
    bool accepted;
    std::string message;
};

std::string toString(TaxiState state);

class TaxiCore
{
public:
    TaxiCore();

    void addZone(const Zone& zone);
    bool hasZone(const std::string& name) const;
    const Zone& zone(const std::string& name) const;

    void setBaseZone(const std::string& name);
    void setReturnToBaseAfterService(bool enabled);

    TaxiRequestResult requestTaxi(const std::string& pickup_zone, const std::string& destination_zone);
    void markNavigationSucceeded();
    void markNavigationFailed(const std::string& reason);
    void markPassengerWaitComplete();
    void markZoneConfirmed(bool confirmed);
    void markVisualObstacle(bool detected);
    void resetToIdle();

    TaxiState state() const;
    TaxiState stateBeforeObstacle() const;
    bool busy() const;
    bool hasActiveGoal() const;
    const Zone& currentGoal() const;
    std::string pickupZone() const;
    std::string destinationZone() const;
    std::string statusMessage() const;
    std::string errorMessage() const;

private:
    void setGoal(const std::string& zone_name);
    void finishService();

    std::map<std::string, Zone> zones_;
    TaxiState state_;
    TaxiState state_before_obstacle_;
    std::string base_zone_;
    std::string pickup_zone_;
    std::string destination_zone_;
    std::string current_goal_;
    std::string status_message_;
    std::string error_message_;
    bool return_to_base_after_service_;
};

}  // namespace mwc_taxibot

#endif  // MWC_TAXIBOT_TAXI_CORE_HPP
