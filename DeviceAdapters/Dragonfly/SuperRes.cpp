#include "SuperRes.h"
#include "Dragonfly.h"

#include <stdexcept>
#include <map>

using namespace std;

const char* const g_SuperResPropertyName = "3D Super Resolution";
const char* const g_SuperResInLightPath = "In light path";
const char* const g_SuperResNotInLightPath = "Not in light path";

CSuperRes::CSuperRes( ISuperResInterface* SuperResInterface, CDragonfly* MMDragonfly )
  : SuperResInterface_( SuperResInterface ),
  MMDragonfly_( MMDragonfly )
{
  // Retrieve values from the device
  if ( !RetrievePositions() )
  {
    throw runtime_error( "Failed to retrieve Super Resolution positions" );
  }

  // Retrieve the current position from the device
  unsigned int vPosition;
  if ( !SuperResInterface_->GetPosition( vPosition ) )
  {
    throw runtime_error( "Failed to read the current Super Resolution position" );
  }

  // Create the MM property
  CPropertyAction* vAct = new CPropertyAction( this, &CSuperRes::OnPositionChange );
  MMDragonfly_->CreateProperty( g_SuperResPropertyName, "Undefined", MM::String, false, vAct );

  // Populate the possible positions
  TPositionNameMap::const_iterator vIt = PositionNames_.begin();
  while ( vIt != PositionNames_.end() )
  {
    MMDragonfly_->AddAllowedValue( g_SuperResPropertyName, vIt->second.c_str() );
    vIt++;
  }

  // Initialise the position
  if ( PositionNames_.find( vPosition ) != PositionNames_.end() )
  {
    MMDragonfly_->SetProperty( g_SuperResPropertyName, PositionNames_[vPosition].c_str() );
  }
  else
  {
    MMDragonfly_->LogComponentMessage( "Current Super Resolution position invalid" );
  }
}

CSuperRes::~CSuperRes()
{
}

bool CSuperRes::RetrievePositions()
{
  bool vPositionsRetrieved = false;
  unsigned int vMinValue, vMaxValue;
  if ( SuperResInterface_->GetLimits( vMinValue, vMaxValue ) )
  {
    for ( unsigned int vIndex = vMinValue; vIndex <= vMaxValue; vIndex++ )
    {
      string vPositionName;
      if ( vIndex == 1 )
      {
        vPositionName = g_SuperResInLightPath;
      }
      else if ( vIndex == 2 )
      {
        vPositionName = g_SuperResNotInLightPath;
      }
      else
      {
        vPositionName = "Undefined Position " + to_string( vIndex );
      }

      PositionNames_[vIndex] = vPositionName;
    }

    vPositionsRetrieved = true;
  }
  return vPositionsRetrieved;
}

int CSuperRes::OnPositionChange( MM::PropertyBase* Prop, MM::ActionType Act )
{
  if ( Act == MM::BeforeGet )
  {
    SetPropertyValueFromDevicePosition( Prop );
  }
  else if ( Act == MM::AfterSet )
  {
    // Search the requested position in the map of existing positions
    string vRequestedPosition;
    Prop->Get( vRequestedPosition );
    bool vFound = false;
    TPositionNameMap::const_iterator vIt = PositionNames_.begin();
    while ( !vFound && vIt != PositionNames_.end() )
    {
      if ( vIt->second == vRequestedPosition )
      {
        vFound = true;
      }
      else
      {
        vIt++;
      }
    }
    if ( vFound )
    {
      // Update device position
      SuperResInterface_->SetPosition( vIt->first );
    }
    else
    {
      // Reset position displayed in the UI to the current device position
      MMDragonfly_->LogComponentMessage( "Unknown Super Resolution position requested [" + vRequestedPosition + "]. Ignoring request." );
      SetPropertyValueFromDevicePosition( Prop );
    }
  }
  return DEVICE_OK;
}

bool CSuperRes::SetPropertyValueFromDevicePosition( MM::PropertyBase* Prop )
{
  bool vValueSet = false;
  unsigned int vPosition;
  if ( SuperResInterface_->GetPosition( vPosition ) )
  {
    if ( PositionNames_.find( vPosition ) != PositionNames_.end() )
    {
      Prop->Set( PositionNames_[vPosition].c_str() );
      vValueSet = true;
    }
    else
    {
      MMDragonfly_->LogComponentMessage( "Current Super Resolution position invalid [ " + to_string(vPosition) + " ]" );
    }
  }
  else
  {
    MMDragonfly_->LogComponentMessage( "Failed to read the current Super Resolution position" );
  }

  return vValueSet;
}
