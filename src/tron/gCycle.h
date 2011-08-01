/*

*************************************************************************

ArmageTron -- Just another Tron Lightcycle Game in 3D.
Copyright (C) 2000  Manuel Moos (manuel@moosnet.de)

**************************************************************************

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

***************************************************************************

*/

#ifndef ArmageTron_CYCLE_H
#define ArmageTron_CYCLE_H

//#define USE_HEADLIGHT
#include <vector>
#include "gStuff.h"
//#include "eSound.h"
//#include "rTexture.h"
//#include "rModel.h"
#include "eNetGameObject.h"
#include "tList.h"
#include "nObserver.h"
#include "rDisplayList.h"

#include "gCycleMovement.h"

class rModel;
class gTextureCycle;
class eSoundPlayer;
class gSensor;
class gNetPlayerWall;
class gPlayerWall;
class eTempEdge;
struct gPredictPositionData;
class gFlagZoneHack;

// minimum time between two cycle turns
extern REAL sg_delayCycle;

// Render the headlight effect?
extern bool headlights;

// steering help
extern REAL sg_rubberCycle;


// this class set is responsible for remembering which walls are too
// close together to pass through safely. The AI uses this information,
// so the real declaration of gCylceMemoryEntry can be found in gAIBase.cpp.
class gCycleMemoryEntry;

class gCycleMemory{
    friend class gCycleMemoryEntry;

    tList<gCycleMemoryEntry>     memory;  // memory about other cylces

public:
    // memory functions: access the memory for a cylce
    gCycleMemoryEntry* Remember(const gCycle *cycle);
    int Len() const {return memory.Len();}
    gCycleMemoryEntry* operator() (int i)  const;
    gCycleMemoryEntry* Latest   (int side)  const;
    gCycleMemoryEntry* Earliest (int side)  const;

    void Clear();

    gCycleMemory();
    ~gCycleMemory();
};

// class used to extrapolate the movement of a lightcycle
class gCycleExtrapolator: public gCycleMovement
{
public:
    void CopyFrom( const gCycleMovement& other );							// copies relevant info from other cylce
    void CopyFrom( const SyncData& sync, const gCycle& other );	        	// copies relevant info from sync data and everything else from other cycle

    gCycleExtrapolator(eGrid *grid, const eCoord &pos,const eCoord &dir,ePlayerNetID *p=NULL,bool autodelete=1);
    // gCycleExtrapolator(nMessage &m);
    virtual ~gCycleExtrapolator();

    // virtual gDestination* GetCurrentDestination() const;			// returns the current destination

    virtual bool EdgeIsDangerous(const eWall *w, REAL time, REAL a) const;

    virtual void PassEdge(const eWall *w,REAL time,REAL a,int recursion=1);

    virtual bool TimestepCore(REAL currentTime, bool calculateAcceleration = true );

    // virtual bool DoTurn(int dir);

    REAL			  trueDistance_;										// distance predicted as best as we can
private:
    // virtual REAL            DoGetDistanceSinceLastTurn  (                               ) const     ;   //!< returns the distance since the last turn

    virtual nDescriptor& CreatorDescriptor() const;

    const gCycleMovement* parent_;												// the cycle that is extrapolated
};

class gCycleChatBot;

#ifndef DEDICATED
class gCycleWallsDisplayListManager
{
    friend class gNetPlayerWall;

public:
    gCycleWallsDisplayListManager();

    //! checks whether a wall at a certain distance can have a display list
    static bool CannotHaveList( REAL distance, gCycle const * cycle );

    void RenderAll( eCamera const * camera, gCycle * cycle );
    bool Walls() const
    {
        return wallList_ || wallsWithDisplayList_;
    }

    void Clear( int inhibit = 0 )
    {
        displayList_.Clear( inhibit );
    }
private:
    gNetPlayerWall *                wallList_;                      //!< linked list of all walls
    gNetPlayerWall *                wallsWithDisplayList_;          //!< linked list of all walls with display list
    rDisplayList                    displayList_;                   //!< combined display list
    REAL                            wallsWithDisplayListMinDistance_; //!< minimal distance of the walls with display list
    int                             wallsInDisplayList_;            //!< number of walls in the current display list
};
#endif

// a class to manage cycle's targets
typedef std::vector<gCycle *> vec_cycle_ptr;
typedef std::vector<gCycle *>::iterator gCycleItr;
class gTarget {
	friend class gCycle;
    gCycle * m_this;
    gCycle * m_target;
    vec_cycle_ptr m_hunters;
    int m_killed_counter;
    REAL m_assignment_time;
    bool started;

public:
    gTarget(gCycle * p_cycle) : m_this(p_cycle), m_target(0), m_killed_counter(0), m_assignment_time(.0), started(0) {}
    ~gTarget() { Unset(); }

    bool Set(gCycle *p_cycle);           // Set a cycle as target. Return true/false for success/failure
    bool Set(ePlayerNetID *p_player);    // Set a player's cycle as target. Return true/false for success/failure
    void Unset();                        // Unset current target
    void Reset()                        // Unset current target and reset counters
    {
        m_killed_counter = 0;
        m_assignment_time = .0;
        Unset();
    }
    bool HasTarget()                     // check whether m_this has a target assigned
    {
        return !m_target;
    }
    bool HasTarget(gCycle *p_cycle)      // check whether p_cycle is m_this assigned target
    {
        return p_cycle==m_target;
    }
    bool IsTargeted()                    // check whether m_this is targeted
    {
        return !m_hunters.empty();
    }
    void Timestep(REAL p_gametime);      // check for timeout
    void AddScore();                     // grant hunter some points
    int  HuntersCount() { return m_hunters.size(); }
    bool Started()      { return started; }

    // Try to set target automatically
    // hint: RANDOM = look for a "random" cycle, FORCE = force p_cycle as target (if possible), EXCLUDE = exclude p_cycle as suitable target
    enum t_hint{RANDOM, FORCE, EXCLUDE};
    bool AutoSet(t_hint p_hint=RANDOM, gCycle *p_cycle=NULL);
    // same on a list of cycles
    static void AutoSetCycles(vec_cycle_ptr &p_cycles, t_hint p_hint=RANDOM, gCycle *p_cycle=NULL);

    // all public static members for settings
    // assignment_mode: 0=disable, 1/2 is enable, 1 affects the player killing your target while 2 randomly affects new target
    static int assignment_mode;
    static int base_score;         // score for 1st target
    static int base_score_deplete; // how the base_score should changed for extra targets
    static int max_target;         // max number of targets for a player
    static int min_cycles;         // min number of cycles required to get another target
    static REAL max_distance;      // max distance to look at first for targets, if no target found, look further ...
    static REAL timeout_delay;     // max time to kill your target
    static REAL start_time;        // time of first target assignment for all cycles
};

// a complete lightcycle
class gCycle: public gCycleMovement
{
    friend class gPlayerWall;
    friend class gNetPlayerWall;
    friend class gDestination;
    friend class gCycleWallRenderer;

    eSoundPlayer *engine;
    eSoundPlayer *turning;
    eSoundPlayer *spark;

    REAL spawnTime_;    //!< time the cycle spawned at
    REAL lastTimeAnim;  //!< last time animation was simulated at
    REAL timeCameIntoView;

    friend class gCycleChatBot;
    std::auto_ptr< gCycleChatBot > chatBot_;

    bool dropWallRequested_; //!< flag indicating that someone requested a wall drop
public:
    static std::vector<gCycle *> cycles;
    static int AliveCounter()
    {
        return std::count_if(cycles.begin(), cycles.end(), std::mem_fun(&gCycle::Alive));
    }

    eCoord            lastGoodPosition_;    // the location of the last known good position

    REAL skew,skewDot;						// leaning to the side

    bool 				mp; 				// use moviepack or not?

    rModel *body,*front,*rear,
    *customModel;

    gTextureCycle  *wheelTex,*bodyTex;
    gTextureCycle  *customTexture;

    eCoord rotationFrontWheel,rotationRearWheel; 	// wheel position (rotation)
    REAL   heightFrontWheel,heightRearWheel;  		// wheel (suspension)

// *** special target mode (begin) ***
private:
    // auto_ptr might not be the best choice here but I don't want to add extra dependencies to use boost::scoped_ptr.
    // The goal here is to have a just-in-time construction and avoid any important memory footprint on gCycle when it's not needed.
    // The cost is the use of auto_ptr and extra check on his internal pointer whether the use of m_target requires it.
    // Side note: never made a copy of m_target as auto_ptr ownership transfer will lead to target management corruption
//    std::auto_ptr<gTarget> m_target_ptr;
// IMPORTANT: auto_ptr has been removed as a workaround to a bug occuring in the following situation:
//            In DEBUG mode, when some1 joined while a lonely player was inside, the round was stopped.
//            If the player entering the server, authenticated when he join (automatically), the server crashed.
//            gdb shows 3 threads all in thread/memory management situations. The one which leads to the crash was desalocated memory
//            within tList and crashed in tMemManager.cpp. Looks like some weird interaction between ZThread, stl memory management
//            and armagetron memory management.
    gTarget m_target_mgr;
public:
    struct LessHuntersCount;
    gTarget &Target() { return m_target_mgr; }
    bool CheckTargetPtr() { return true; }
// *** special target mode (end) ***

public:
    //REAL	brakingReservoir; // reservoir for braking. 1 means full, 0 is empty

    gFlagZoneHack *flag_;

    REAL lastShotTime;
    REAL shotStarted;
    REAL shotReservoir;
    void ProcessShoot(bool deathShot);
    void Killed(gCycle *pKiller, int type = 0);

    static uActionPlayer s_brake;
    gCycleMemory memory;

    gRealColor color_;
    gRealColor trailColor_;

    // smooth corrections
    // pos is always the correct simulated position; the displayed position is calculated as pos + correctPosSmooth
    // and correctPosSmooth decays with time.
    eCoord correctPosSmooth;
    eCoord predictPosition_; //!< the best guess of where the cycle is at at display time

    // every frame, a bit of this variable is taken away and added to the step the cycle makes.
    REAL correctDistanceSmooth;

private:
    void TransferPositionCorrectionToDistanceCorrection();

#ifndef DEDICATED
    gCycleWallsDisplayListManager displayList_;                     //!< display list manager
#endif

    tCHECKED_PTR(gNetPlayerWall)	currentWall;                    //!< the wall that currenly is attached to the cycle
    tCHECKED_PTR(gNetPlayerWall)	lastWall;                       //!< the last wall that was attached to this cycle
    tCHECKED_PTR(gNetPlayerWall)	lastNetWall;                    //!< the last wall received over the network

    // for network prediction
    SyncData									lastSyncMessage_;	// the last sync message the cycle received
    tJUST_CONTROLLED_PTR<gCycleExtrapolator>	extrapolator_;		// the cycle copy used for extrapolation
    bool										resimulate_;		// flag indicating that a new extrapolation should be started

    void	ResetExtrapolator();							// resets the extrapolator to the last known state
    bool	Extrapolate( REAL dt );							// simulate the extrapolator at higher speed
    void	SyncFromExtrapolator();							// take over the extrapolator's data

    virtual void OnNotifyNewDestination(gDestination *dest);   //!< called when a destination is successfully inserted into the destination list
    virtual void OnDropTempWall        ( gPlayerWall * wall, eCoord const & position, eCoord const & direction );   //!< called when another cycle grinds a wall; this cycle should then drop its current wall if the grinding is too close.

    //	unsigned short currentWallID;

    nTimeRolling nextSync, nextSyncOwner;
    REAL lastSyncOwnerGameTime_;    //!< time of the last sync to the owner in game time

    void MyInitAfterCreation();

    void SetCurrentWall(gNetPlayerWall *w);

    void PreparePredictPosition( gPredictPositionData & data ); //!< prepares CalculatePredictPosition() call, requesting a raycast to the front
    REAL CalculatePredictPosition( gPredictPositionData & data ); //!< Calculates predictPosition_
protected:
    virtual ~gCycle();

    virtual void OnRemoveFromGame(); // called when the cycle is physically removed from the game

    virtual void OnRoundEnd();    //!< called when the round ends

    // virtual REAL            DoGetDistanceSinceLastTurn  (                               ) const     ;   //!< returns the distance since the last turn
public:
    virtual void Die ( REAL time )  ;  //!< dies at the specified time
    void KillAt( const eCoord& pos );  //!< kill this cycle at the given position and take care of scoring

    int WindingNumber() const {return windingNumber_;}

    virtual bool            Vulnerable              ()                                    const     ;   //!< returns whether the cycle can be killed

    // bool CanMakeTurn() const { return pendingTurns <= 0 && lastTime >= nextTurn; }

    virtual void InitAfterCreation();
    gCycle(eGrid *grid, const eCoord &pos,const eCoord &dir,ePlayerNetID *p=NULL);

    static	void 	SetWallsStayUpDelay		( REAL delay );				//!< the time the cycle walls stay up ( negative values: they stay up forever )
    static	void 	SetWallsLength			( REAL length);				//!< the maximum total length of the walls
    static	void 	SetExplosionRadius		( REAL radius);				//!< the radius of the holes blewn in by an explosion

    static	REAL 	WallsStayUpDelay()	 { return wallsStayUpDelay;	}	//!< the time the cycle walls stay up ( negative values: they stay up forever )
    static	REAL	WallsLength()	 	 { return wallsLength;		}	//!< the default total length of the walls
    REAL	        MaxWallsLength() const;                             //!< the maximum total length of the walls (including max effect of rubber growth)
    REAL	        ThisWallsLength() const;                            //!< the maximum total length of this cycle's wall (including rubber shrink)
    REAL	        WallEndSpeed() const;                               //!< the speed the end of the trail is receeding with right now
    static	REAL	ExplosionRadius()	 { return explosionRadius;	}	//!< the radius of the holes blewn in by an explosion

    bool    IsMe( eGameObject const * other ) const;              //!< checks whether the passed pointer is logically identical with this cycle

    // the network routines:
    gCycle(nMessage &m);
    virtual void WriteCreate(nMessage &m);
    virtual void WriteSync(nMessage &m);
    virtual void ReadSync(nMessage &m);
    virtual void RequestSyncOwner(); //!< requests special syncs to the owner on important points (just passed an enemy trail end safely...)
    virtual void RequestSyncAll(); //!< requests special syncs to everyone on important points (just passed an enemy trail end safely...)

    virtual void SyncEnemy ( const eCoord& begWall );    //!< handle sync message for enemy cycles
    // virtual void SyncFriend( const eCoord& begWall );    //!< handle sync message for enemy cycles

    virtual void ReceiveControl(REAL time,uActionPlayer *Act,REAL x);
    virtual void PrintName(tString &s) const;
    virtual bool ActionOnQuit();

    virtual nDescriptor &CreatorDescriptor() const;
    virtual bool SyncIsNew(nMessage &m);
    //virtual bool ClearToTransmit(int user) const;

    virtual bool Timestep(REAL currentTime);
    virtual bool TimestepCore(REAL currentTime,bool calculateAcceleration = true);

    virtual void InteractWith(eGameObject *target,REAL time,int recursion=1);

    virtual bool EdgeIsDangerous(const eWall *w, REAL time, REAL a) const;

    virtual void PassEdge(const eWall *w,REAL time,REAL a,int recursion=1);

    virtual REAL PathfindingModifier( const eWall *w ) const;

    virtual bool Act(uActionPlayer *Act,REAL x);

    virtual bool DoTurn(int dir);
    void DropWall( bool buildNew=true );                                    //!< Drops the current wall and builds a new one

    // void Turbo(bool turbo);

    virtual void Kill();

    const eTempEdge* Edge();
    const gPlayerWall* CurrentWall();
    // const gPlayerWall* LastWall();

	void TeleportTo(eCoord dest, eCoord dir, REAL time);
    void SetWallBuilding(bool build);

#ifndef DEDICATED
    virtual void Render(const eCamera *cam);

    virtual void RenderName( const eCamera *cam );

    virtual bool RenderCockpitFixedBefore(bool primary=true);

    virtual void SoundMix(unsigned char *dest,unsigned int len,
                          int viewer,REAL rvol,REAL lvol);
#endif

	//! draws it in a svg file
	virtual void DrawSvg(std::ofstream &f);

    virtual eCoord CamPos() const;
    virtual eCoord PredictPosition() const;
    virtual eCoord  CamTop() const;

    virtual void RightBeforeDeath( int numTries );

#ifdef POWERPAK_DEB
    virtual void PPDisplay();
#endif

    static 	void	PrivateSettings();									// initiate private setting items

    //	virtual void AddRef();
    //	virtual void Release();

private:
    static	REAL	wallsStayUpDelay;			//!< the time the cycle walls stay up ( negative values: they stay up forever )
    static	REAL	wallsLength;				//!< the maximum total length of the walls
    static	REAL	explosionRadius;			//!< the radius of the holes blewn in by an explosion

protected:
    virtual 	bool 			DoIsDestinationUsed		( const gDestination *	dest		) const		;	//!< returns whether the given destination is in active use
};

#endif

