/* FILENAME	 : TerrainNav.cpp
 * AUTHOR	 : Debbie Meduna
 * DATE 	 : 04/27/09
 *
 * LAST MODIFIED : 11/30/10
 * MODIFIED BY	 : Debbie Meduna
 * -----------------------------------------------------------------------------
 * Modification History
 * -----------------------------------------------------------------------------
 ******************************************************************************/

#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include "TerrainNav.h"
#include "TerrainNavLog.h"

#define MBTRN_DEBUG 0

/*TODO delete Transition Matrix, filterState, and newState
trn_server.cpp calls getFilterState() which currently only returns 0
*/
/******************************************************************************
 TRANSITION MATRIX
 State 0: Well localized
 State 1: Localizing - medium uncertainty
 State 2: High uncertainty

 Trigger 0: North/East Variance < MIN_FILTER_VAR
 Trigger 1: MIN_FILTER_VAR+VAR_MARGIN < North/East Variance < MAX_FILTER_VAR-
	    VAR_MARGIN and filter is either in state 0 or 2
 Trigger 2: North/East Variance > MAX_FILTER_VAR
 Trigger 3: Missing valid range measurements for > MAX_MEAS_OUTAGE OR
	    Vehicle does not have bottom lock for > MAX_VEL_OUTAGE
 Trigger 4: Vehicle has been over flat terrain for too long
 Trigger 5: Windowed NIS maximum has been exceeded
******************************************************************************/
//static int transitionMatrix[5][3] = {{0, 0, 1}, {1, 1, 1}, {1, 2, 2}, {2, 2, 2}, {2, 2, 2}};
TerrainNav::TerrainNav()
: tNavFilter(NULL),allowFilterReinits(true),numReinits(0),
saveDirectory(NULL), vehicleSpecFile(NULL),
particlesFile(NULL), mapFile(NULL),
filterType(1),mapType(1),
terrainMap(NULL),_trnLog(NULL)
{

}
TerrainNav::TerrainNav(char* mapName) {
	//initialize pointers
	this->mapFile = STRDUPNULL(mapName);
	this->vehicleSpecFile = (char*)"mappingAUV_specs.cfg";
	this->particlesFile = NULL;
	this->saveDirectory = NULL;
	this->tNavFilter = NULL;
	this->terrainMap = NULL;
	this->filterType = 1;
	//this->octreeMap = NULL;
	this->mapType = 1;  // defaults to DEM
	this->allowFilterReinits = true;

	if(this->mapType == 1) {
		terrainMap = new TerrainMapDEM(this->mapFile);
	} else {
		terrainMap = new TerrainMapOctree(this->mapFile);
	}
	//initialize terrainNav private variables
	initVariables();

}

TerrainNav::TerrainNav(char* mapName, char* vehicleSpecs) {
	//initialize pointers
	this->mapFile = STRDUPNULL(mapName);
	this->vehicleSpecFile = STRDUPNULL(vehicleSpecs);
	this->particlesFile = NULL;
	this->saveDirectory = NULL;
	this->tNavFilter = NULL;
	this->terrainMap = NULL;
	this->filterType = 1;
	//this->octreeMap = NULL;
	this->mapType = 1;
	this->allowFilterReinits = true;

	if(this->mapType == 1) {
		this->terrainMap = new TerrainMapDEM(this->mapFile);
	} else {
		this->terrainMap = new TerrainMapOctree(this->mapFile);
	}

	//initialize terrainNav private variables
	initVariables();
}

TerrainNav::TerrainNav(char* mapName, char* vehicleSpecs,
					   const int& filterType) {
	//initialize pointers
	this->mapFile = STRDUPNULL(mapName);
	this->vehicleSpecFile = STRDUPNULL(vehicleSpecs);
	this->particlesFile = NULL;
	this->saveDirectory = NULL;
	this->tNavFilter = NULL;
	this->terrainMap = NULL;
	this->filterType = filterType;
	//this->octreeMap = NULL;
	this->mapType = 1;
	this->allowFilterReinits = true;

	if(this->mapType == 1) {
		this->terrainMap = new TerrainMapDEM(this->mapFile);
	} else {
		this->terrainMap = new TerrainMapOctree(this->mapFile);
	}

	//initialize terrainNav private variables
	initVariables();
}

TerrainNav::TerrainNav(char* mapName, char* vehicleSpecs,
					   const int& filterType, char* directory) {
	//initialize pointers
	this->mapFile = STRDUPNULL(mapName);
	this->vehicleSpecFile = STRDUPNULL(vehicleSpecs);
	this->saveDirectory = STRDUPNULL(directory);
	this->particlesFile = NULL;
	this->tNavFilter = NULL;
	this->terrainMap = NULL;
	this->filterType = filterType;
	//this->octreeMap = NULL;
	this->mapType = 1;
	this->allowFilterReinits = true;

	if(this->mapType == 1) {
		this->terrainMap = new TerrainMapDEM(this->mapFile);
	} else {
		this->terrainMap = new TerrainMapOctree(this->mapFile);
	}

	//initialize terrainNav private variables
	initVariables();
}

TerrainNav::TerrainNav(char* mapName, char* vehicleSpecs,
					   const int& filterType, const int& mapType) {
	//initialize pointers
	this->mapFile = STRDUPNULL(mapName);
	this->vehicleSpecFile = STRDUPNULL(vehicleSpecs);
	this->saveDirectory = NULL;
	this->particlesFile = NULL;
	this->tNavFilter = NULL;
	this->filterType = filterType;
	//this->octreeMap = NULL;
	this->mapType = mapType;
	this->allowFilterReinits = true;

	if(this->mapType == 1) {
		this->terrainMap = new TerrainMapDEM(this->mapFile);
	} else {
		this->terrainMap = new TerrainMapOctree(this->mapFile);
	}

	//initialize terrainNav private variables
	initVariables();
}

TerrainNav::TerrainNav(char* mapName, char* vehicleSpecs,
					   const int& filterType, const int& mapType, char* directory) {
	//initialize pointers
	this->mapFile = STRDUPNULL(mapName);
	this->vehicleSpecFile = STRDUPNULL(vehicleSpecs);
	this->saveDirectory = STRDUPNULL(directory);
	this->particlesFile = NULL;
	this->tNavFilter = NULL;
	this->terrainMap = NULL;
	this->filterType = filterType;
	//this->octreeMap = NULL;
	this->mapType = mapType;
	this->allowFilterReinits = true;

	if(this->mapType == 1) {
		this->terrainMap = new TerrainMapDEM(this->mapFile);
	} else {
		this->terrainMap = new TerrainMapOctree(this->mapFile);
	}

	//initialize terrainNav private variables
	initVariables();
}

TerrainNav::TerrainNav(char* mapName, char* vehicleSpecs, char* particles,
							  const int& filterType, const int& mapType, char* directory) {

	//initialize pointers
	this->mapFile = STRDUPNULL(mapName);
	this->vehicleSpecFile = STRDUPNULL(vehicleSpecs);
	this->saveDirectory = STRDUPNULL(directory);
	this->particlesFile = STRDUPNULL(particles);
	this->tNavFilter = NULL;
	this->terrainMap = NULL;
	this->filterType = filterType;
	//this->octreeMap = NULL;
	this->mapType = mapType;
	this->allowFilterReinits = true;

	if(this->mapType == 1) {
		this->terrainMap = new TerrainMapDEM(this->mapFile);
	} else {
		this->terrainMap = new TerrainMapOctree(this->mapFile);
	}

	//initialize terrainNav private variables
	initVariables();
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Constructor finished.\n");
}

TerrainNav::~TerrainNav() {
	if(tNavFilter != NULL) {
		delete tNavFilter;
	}
	tNavFilter = NULL;

	if(this->terrainMap != NULL){
		delete this->terrainMap;
	}
	this->terrainMap = NULL;

	if(this->mapFile !=NULL)free(this->mapFile);
	this->mapFile=NULL;

	if(this->vehicleSpecFile != NULL)free(this->vehicleSpecFile);
	this->vehicleSpecFile=NULL;
	if(this->saveDirectory !=NULL)free(this->saveDirectory);
	this->saveDirectory=NULL;
	if(this->particlesFile !=NULL)free(this->particlesFile);
	this->particlesFile=NULL;

//	if (_posLog) delete _posLog;

	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Number of reinitializations: %i\n", numReinits);
}

void TerrainNav::estimatePose(poseT* estimate, const int& type) {
	//Cannot compute pose estimates if the filter motion has not been initialized
	if(tNavFilter->lastNavPose == NULL) {
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Cannot compute pose estimate; motion has not been initialized.\n");
		return;
	}

	switch(type) {
	   case 2:      //Compute MMSE
			tNavFilter->computeMMSE(estimate);

			//      Use the vn slots to pass the Kearfott's location at the time the measurement was taken.
			//
			estimate->time = tNavFilter->lastNavPose->time;
			estimate->vn_x = tNavFilter->lastNavPose->x;
			estimate->vn_y = tNavFilter->lastNavPose->y;
			estimate->vn_z = tNavFilter->lastNavPose->z;


			//If using a PMF, add on prior estNavOffset for attitude
			if(this->filterType == 1 && ALLOW_ATTITUDE_SEARCH) {
				estimate->phi = tNavFilter->lastNavPose->phi + estNavOffset.phi;
				estimate->theta = tNavFilter->lastNavPose->theta + estNavOffset.theta;
				estimate->psi = tNavFilter->lastNavPose->psi + estNavOffset.psi;
				estimate->wy = tNavFilter->lastNavPose->wy + estNavOffset.wz;
				estimate->wz = tNavFilter->lastNavPose->wz + estNavOffset.wz;
			}
			//Update current filter North/East variance
			tNavFilter->currVar[0] = estimate->covariance[0];
			tNavFilter->currVar[1] = estimate->covariance[2];
			//If estimate is confident, save INS predicted offset for reinit
			if(tNavFilter->currVar[0] < 100.0 && tNavFilter->currVar[1] < 100.0) {
				this->estNavOffset = *estimate;
				this->estNavOffset -= *this->tNavFilter->lastNavPose;
			}
			_trnLog->logMMSE(estimate);      // Record the MMSE estimate
			_trnLog->logNav(&_incomingNav);
			_trnLog->logReinits( numReinits );
			_trnLog->write();

			break;

	   case 1:      //Compute MLE
	   default:
			tNavFilter->computeMLE(estimate);
			_trnLog->logMLE(estimate);       // Record the MLE estimate
			break;
	}

	//TODO:***************NEED TO FIGURE THIS OUT!!!!! *********************
	//Trying to solve problem of TRN estimate latching to a specific value and not
	//changing with motion updates
	//*estimate = *tNavFilter->lastNavPose;
	//*estimate += this->estNavOffset;
	////*estimate += estNavOffset;

	if(std::isnan(estimate->x) || std::isnan(estimate->y)){
		estimate->covariance[0] = X_STDDEV_INIT*X_STDDEV_INIT;
		estimate->covariance[2] = Y_STDDEV_INIT*Y_STDDEV_INIT;
	}

	/* Log to a data file */
   logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"\n variances([0], [2], and psi_berg) = "
   	"%f  %f  %f\n",
   	estimate->covariance[0],
   	estimate->covariance[2],
    estimate->covariance[44]);

	return;
}

void TerrainNav::measUpdate(measT* incomingMeas, const int& type) {
	//copy incoming measurement to current meas structure;
	measT currMeas;
	int i;
    // measT '=' operator performs copy operation,
    // allocating dynamic memory for variables...
    // this has to be released (note call to clean() below)
	currMeas = *incomingMeas;

	currMeas.dataType = type;

  // Record measurement
  if (_trnLog) _trnLog->logMeas(&currMeas);

	//check validity of range data
	checkRangeValidity(currMeas);

	//If no motion updates have been performed (no navigation estimates included)
	//the measurement can not be added.
	if(tNavFilter->lastNavPose == NULL) {
		//check if our current measurements are valid
		this->lastMeasValid = false;
		for(i = 0; i < currMeas.numMeas; i++) {
			if(currMeas.measStatus[i]) {
				this->lastMeasValid = true;
				break;
			}
		}

		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Measurement type %i from time = %.2f sec. not included; "
			   "vehicle motion has not been initialized. lastMeasValid = %d, beams = %d, ping # %u\n", currMeas.dataType,
		       currMeas.time, lastMeasValid, currMeas.numMeas, currMeas.ping_number);
		this->lastMeasSuccess = false;
	// release dynamic memory resources
		currMeas.clean();

		return;
	}

	//check if vehicle is within correlation map before including measurement
	if(this->mapType == 1) {
		if(!this->tNavFilter->withinRefMap()) {
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Measurement type %i from time = %.2f sec, ping # %u"
				   " not included; vehicle is operating outside the given reference maps.\n",
				   currMeas.dataType, currMeas.time, currMeas.ping_number);
			this->lastMeasSuccess = false;
			return;
		}
	}

	//Fill in the measurement variance based on range percent error
	computeMeasVariance(currMeas);

	//If the current measurement time is ahead of the latest navigation time,
	//then we need to wait for more recent navigation data before adding the
	//measurement.
	if((tNavFilter->lastNavPose->time < currMeas.time)) {
		//add current measurement to the measurement buffer
	// measT '=' operator performs copy operation,
	// allocating dynamic memory for variables...
	// this has to be released (after pending measurements processed)
		this->waitingMeas[this->numWaitingMeas] = currMeas;
		this->numWaitingMeas++;

		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Delayed incorporating measurement type %i from time = %.2f"
			   " sec, ping # %u; waiting for more recent INS data...\n",
			   currMeas.dataType, currMeas.time, currMeas.ping_number);
	// release dynamic memory resources
		currMeas.clean();
		return;
	}

	//If the current navigation time matches the measurement time, add the
	//measurement.	Otherwise, ignore the measurement.
	if(tNavFilter->lastNavPose->time == currMeas.time) {
		this->lastMeasSuccess = tNavFilter->measUpdate(currMeas);
		if(this->lastMeasSuccess) {
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::measUpdate -  Measurement type %i successfully incorporated from"
				   " time = %.2f sec, ping # %u.\n",
				   currMeas.dataType, currMeas.time, currMeas.ping_number);
			lastMeasSuccessTime = currMeas.time;
		}
	// release dynamic memory resources
		currMeas.clean();

		return;
	} else{
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Did not incorporate measurement type %i from time"
			   "= %.2f sec, ping # %u; no INS pose data available. \n",
			   currMeas.dataType, currMeas.time, currMeas.ping_number);
	}
    // release dynamic memory resources
	currMeas.clean();

	return;
}

		//////////////////////////

void TerrainNav::motionUpdate(poseT* incomingNav) {

	// Log position data
	//
//	_posLog->log(incomingNav);

	// Maintain a copy of the latest incoming nav
	//
	_incomingNav = *incomingNav;

	poseT currEstimate;
	double dt;

   // make a copy
	currEstimate = *incomingNav;
#if MBTRN_DEBUG
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT time:%.3f\n", currEstimate.time);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT  phi:%.3f\n", currEstimate.phi);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT thta:%.3f\n", currEstimate.theta);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT  psi:%.3f\n", currEstimate.psi);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT dvlV:%d\n", currEstimate.dvlValid);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT gpsV:%d\n", currEstimate.gpsValid);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT bttm:%d\n", currEstimate.bottomLock);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT    x:%.3f\n", currEstimate.x);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT    y:%.3f\n", currEstimate.y);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT    z:%.3f\n", currEstimate.z);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT   vx:%.3f\n", currEstimate.vx);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT   vy:%.3f\n", currEstimate.vy);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT   vz:%.3f\n", currEstimate.vz);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT   wx:%.3f\n", currEstimate.wx);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT   wy:%.3f\n", currEstimate.wy);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"poseT   wz:%.3f\n", currEstimate.wz);
#endif
	/* Log to a data file */
   logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::motionUpdate() - currEstimate = "
      	"(%.f, %.2f, %.2f)\n", currEstimate.x, currEstimate.y, currEstimate.z);

	//try to initialize the filter if not already initialized
	if(tNavFilter->lastNavPose == NULL) {
		attemptInitFilter(currEstimate);
		return;
	}

	//check filter health before applying next motion update
	if(this->allowFilterReinits && !checkFilterHealth()) {
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::motionUpdate() - filter unhealthy, skipping this update\n");
		return;
	}

	//estimate current acceleration based on delta v
	dt = currEstimate.time - tNavFilter->timeLastDvlValid;

	if( INTG_POS )
	{
	   //Convert currEstimate->pose (.x, .y .z) from body to iceberg.  psi_berg.
	   Matrix currVelBody(3,1);
	   Matrix prevVelBody(3,1);
	   Matrix currVelMap(3,1);
	   Matrix prevVelMap(3,1);

	   double prevAttitude[3] = {tNavFilter->lastNavPose->phi, tNavFilter->lastNavPose->theta,
				     tNavFilter->lastNavPose->psi};

	   double currAttitude[3] = {currEstimate.phi, currEstimate.theta,
				     currEstimate.psi};

	   prevVelBody(1,1) = tNavFilter->lastNavPose->vx;
	   prevVelBody(2,1) = tNavFilter->lastNavPose->vy;
	   prevVelBody(3,1) = tNavFilter->lastNavPose->vz;

	   currVelBody(1,1) = currEstimate.vx;
	   currVelBody(2,1) = currEstimate.vy;
	   currVelBody(3,1) = currEstimate.vz;

	   logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::motionUpdate() - currEstimate = "
		"(%.f, %.2f, %.2f)\n", currEstimate.x, currEstimate.y, currEstimate.z);


	   prevVelMap = tNavFilter->applyRotation( prevAttitude, prevVelBody );

	   currVelMap = tNavFilter->applyRotation( currAttitude, currVelBody );

	   double deltat = currEstimate.time - tNavFilter->lastNavPose->time;

//         Trapezoidal:
//	   currEstimate.x = tNavFilter->lastNavPose->x + (prevVelMap(1,1) + currVelMap(1,1))*deltat/2.;
//	   currEstimate.y = tNavFilter->lastNavPose->y + (prevVelMap(2,1) + currVelMap(2,1))*deltat/2.;
//	   currEstimate.z = tNavFilter->lastNavPose->z + (prevVelMap(3,1) + currVelMap(3,1))*deltat/2.;
//
//         Forward Euler:
	   currEstimate.x = tNavFilter->lastNavPose->x + (prevVelMap(1,1) )*deltat;
	   currEstimate.y = tNavFilter->lastNavPose->y + (prevVelMap(2,1) )*deltat;
	   currEstimate.z = tNavFilter->lastNavPose->z + (prevVelMap(3,1) )*deltat;

	   logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::motionUpdate() - integrated currEstimate = "
		"(%.2f, %.2f, %.2f)\n", currEstimate.x, currEstimate.y, currEstimate.z);
	}

	if(dt > 0) {
		currEstimate.ax = (currEstimate.vx - lastValidVel[0]) / dt;
		currEstimate.ay = (currEstimate.vy - lastValidVel[1]) / dt;
		currEstimate.az = (currEstimate.vz - lastValidVel[2]) / dt;
	}

	//check validity of velocity data
	checkVelocityValidity(currEstimate);
	if(currEstimate.bottomLock && currEstimate.dvlValid) {
		lastBottomLockTime = currEstimate.time;
	}
	else
	{
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::motionUpdate() - Invalid velocity\n");
	}

	//if using a compass bias correction function, apply here:
	if(tNavFilter->compassBias != NULL) {
		currEstimate.psi += -tNavFilter->compassBias->evalCompassBias(currEstimate.psi);
	}

	//if dvl velocity data is bad, use last good velocity info
	if(!currEstimate.dvlValid) {
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::motionUpdate() - Invalid velocity, using prior value\n");
		currEstimate.vx = lastValidVel[0];
		currEstimate.vy = lastValidVel[1];
		currEstimate.vz = lastValidVel[2];
		currEstimate.bottomLock = lastVelBotLock;
	} else {
		//if we just lost bottom lock, reset water current velocity estimate
		if(lastVelBotLock && !currEstimate.bottomLock) {
			poseT currEst;
			tNavFilter->computeMMSE(&currEst);
			double attitude[3] = {currEst.phi, currEst.theta,
								  currEst.psi
								 };
			//assuming attitude is ~constant over two time steps, can first
			//compute estimated current velocity in the body frame and then rotate
			//into inertial.  (v_c = v_w - v_b, where v_c is current velocity,
			//v_w is water-relative velocity, v_b is bottom-relative velocity)

			Matrix estWatVel(3, 1);
			Matrix tempCurrentVel(3, 1);
			estWatVel(1, 1) = currEstimate.vx - lastValidVel[0];
			estWatVel(2, 1) = currEstimate.vy - lastValidVel[1];
			estWatVel(3, 1) = currEstimate.vz - lastValidVel[2];

			tempCurrentVel = tNavFilter->applyRotation(attitude, estWatVel);
			tNavFilter->currentVel[0] = tempCurrentVel(1, 1);
			tNavFilter->currentVel[1] = tempCurrentVel(2, 1);
			tNavFilter->currentVel[2] = tempCurrentVel(3, 1);

		}
		/* Log velocities to a data file */
		lastValidVel[0] = currEstimate.vx;
		lastValidVel[1] = currEstimate.vy;
		lastValidVel[2] = currEstimate.vz;
		lastVelBotLock = currEstimate.bottomLock;
		tNavFilter->timeLastDvlValid = currEstimate.time;
	}

	/*
	//convert measured velocity to vehicle frame (account for vehicle rotation
	//rate)
	int sensorIndx = 0;
	//look for the dvl sensor in the vehicle info
	if(tNavFilter->findMeasSensorIndex(1, sensorIndx))
	{
	   double rx, ry, rz;
	   rx = tNavFilter->vehicle->T_sv[sensorIndx].translation[0];
	   ry = tNavFilter->vehicle->T_sv[sensorIndx].translation[1];
	   rz = tNavFilter->vehicle->T_sv[sensorIndx].translation[2];

	   currEstimate->vx += currEstimate->wy*rz - currEstimate->wz*ry;
	   currEstimate->vy += currEstimate->wz*rx - currEstimate->wx*rz;
	   currEstimate->vz += currEstimate->wx*ry - currEstimate->wy*rx;
	   }*/


	//if measurement is waiting to be added, update motion and add measurement
	if(outstandingMeas()) {
		//interpolate to find navigation corresponding to measurement
		poseT measPose;
		if(interpolatePoses(*tNavFilter->lastNavPose, currEstimate, measPose,
							waitingMeas[numWaitingMeas - 1].time)) {
			for(int i = 0; i < this->numWaitingMeas; i++) {
				interpolatePoses(*tNavFilter->lastNavPose, currEstimate, measPose,
								 waitingMeas[i].time);

				//check that we are not interpolating over a large time difference
				if(measPose.time - tNavFilter->lastNavPose->time > MAX_INTERP_TIME
						|| currEstimate.time - measPose.time > MAX_INTERP_TIME) {
					this->lastMeasSuccess = false;
					logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Measurement type %i not incorporated from time = "
						   "%.2f sec.; No relevant navigation data available\n",
						   waitingMeas[i].dataType, measPose.time);
				} else {
					//perform motion update in navigation filter
					tNavFilter->motionUpdate(measPose);

					//update lastNavPose variable
					*tNavFilter->lastNavPose = measPose;

					//incorporate measurement
					this->lastMeasSuccess = tNavFilter->measUpdate(waitingMeas[i]);

					if(this->lastMeasSuccess) {
						logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::motionUpdate - Measurement type %i successfully incorporated "
							   "from time = %.2f sec.\n", waitingMeas[i].dataType, measPose.time);
						lastMeasSuccessTime = measPose.time;
					}
				}
			}

			// release resources allocated earlier by copy operator
			for(int i = 0; i < this->numWaitingMeas; i++) {
		    waitingMeas[i].clean();
	    }
			//set numWaitingMeas to zero because all measurements are included
			this->numWaitingMeas = 0;
		}
	}

	//perform motion update in navigation filter
	tNavFilter->motionUpdate(currEstimate);

	//update lastNavPose variable
	*tNavFilter->lastNavPose = currEstimate;

	return;
}

void TerrainNav::createFilter(const int filterType, const double* windowVar) {
	//ensure that the filter object is empty before creating.
	if(tNavFilter != NULL) {
		delete tNavFilter;
	}
	tNavFilter = NULL;

	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"Loading vehicle config file... %s\n", this->vehicleSpecFile);

	//create new filter based on given filter type
	switch(filterType) {
		case 1:
			tNavFilter = new TNavPointMassFilter(this->terrainMap, this->vehicleSpecFile,
												 this->saveDirectory, windowVar, this->mapType);
			break;

		case 2:
			tNavFilter = new TNavParticleFilter(this->terrainMap, this->vehicleSpecFile,
												this->saveDirectory, windowVar, this->mapType);
			break;
		case 3:
			tNavFilter = new TNavBankFilter(this->terrainMap, this->vehicleSpecFile,
												this->saveDirectory, windowVar, this->mapType);
			break;
		default:
			tNavFilter = new TNavPointMassFilter(this->terrainMap, this->vehicleSpecFile,
												 this->saveDirectory, windowVar, this->mapType);
	}
	this->filterType = filterType;
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::TNavFilter initialized with type %i\n", filterType);

}



void TerrainNav::initVariables() {
	_initialized = false;
	bool mapOk = true;

	//initialize default variance for window size
	double windowVar[N_COVAR] =
			{X_STDDEV_INIT * X_STDDEV_INIT,
			 0.0, Y_STDDEV_INIT * Y_STDDEV_INIT,
			 0.0, 0.0, Z_STDDEV_INIT * Z_STDDEV_INIT,
			 0.0, 0.0, 0.0, PHI_STDDEV_INIT * PHI_STDDEV_INIT,
			 0.0, 0.0, 0.0, 0.0, THETA_STDDEV_INIT * THETA_STDDEV_INIT,
			 0.0, 0.0, 0.0, 0.0, 0.0, PSI_STDDEV_INIT * PSI_STDDEV_INIT,
			 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, GYRO_BIAS_STDDEV_INIT * GYRO_BIAS_STDDEV_INIT,
			 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, GYRO_BIAS_STDDEV_INIT * GYRO_BIAS_STDDEV_INIT,
			 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, PSI_BERG_STDDEV_INIT * PSI_BERG_STDDEV_INIT
						   };

	copyToLogDir();

	//create log objects
	_trnLog = new TerrainNavLog(DataLog::BinaryFormat);
  tl_new_logfile(dirname(STRDUPNULL(_trnLog->fileName())));

	// Initialize TNavConfig
	TNavConfig::instance()->setMapFile(this->mapFile);
	TNavConfig::instance()->setVehicleSpecsFile(this->vehicleSpecFile);
	TNavConfig::instance()->setParticlesFile(this->particlesFile);
	TNavConfig::instance()->setLogDir(this->saveDirectory);

	//
	// Shut off measWeights.txt debug.  It is too large.
	this->saveDirectory = NULL;


	//logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::TNavFilter initialized with Z variance %.2f\n", windowVar[5]);

	//create filter object
	createFilter(this->filterType, windowVar);

	//old state machine version: filterState = 1;
	//filterState = 2;//1;

	int i;
	lastMeasSuccess = false;
	numWaitingMeas = 0;
	lastValidVel[0] = 0.0;
	lastValidVel[1] = 0.0;
	lastValidVel[2] = 0.0;
	lastVelBotLock = false;
	lastMeasValid = false;
	lastMeasSuccessTime = -1.0;
	lastInitAttemptTime = -1.0;
	lastBottomLockTime = -1.0;
	for(i = 0; i < 4; i++) {
		lastValidRange[i] = 0;
		lastValidRangeTime[i] = 0;
		noValidRange[i] = true;
	}
	numReinits = 0;
	useModifiedWeighting = TRN_WT_NONE;

	_initialized = mapOk;
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::initVariables finished.\n");

}

void TerrainNav::attemptInitFilter(poseT& initEstimate) {
	bool withinMap;
	int i;
	static double windowVarInc[N_COVAR] = {
					  0.0,
					  0.0, 0.0,
					  0.0, 0.0, 0.0,
					  0.0, 0.0, 0.0, 0.0,
					  0.0, 0.0, 0.0, 0.0, 0.0,
					  0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
					  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
					  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
					  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
									 };

	if(this->mapType == 1) {
		withinMap = tNavFilter->withinValidMapRegion(initEstimate.x, initEstimate.y);
	} else {
		withinMap = true;  // TODO:**** Need to actually check within map, if we care for octrees ********
	}
	checkVelocityValidity(initEstimate);

	//initialize the filter if not already initialized AND if within valid
	//region of the map AND vehicle has bottom lock AND vehicle has good
	//measurements AND vehicle is not on the surface
        //
        // Added a switch to essentially skip the gpsValid check
        //
	if(withinMap && initEstimate.bottomLock && lastMeasValid &&
	    initEstimate.dvlValid && initEstimate.z > 1 &&
            (!initEstimate.gpsValid || TNavConfig::instance()->getIgnoreGps())) {
		//Incorporate increased search window to account for large initialization
		//waiting times
		for(i = 0; i < N_COVAR; i++) {
			windowVarInc[i] *= windowVarInc[i];
		}
		tNavFilter->increaseInitSearchWin(windowVarInc);

		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::attemptInitFilter is increasing Init Search Window by %f m\n", windowVarInc[0]);
		for(i = 0; i < N_COVAR; i++) {
			windowVarInc[i] = 0.0;
		}
		//lastInitAttemptTime = -1;

		//Initialize vehicle motion
		initMotion(initEstimate);
	} else {
		//increase search region for filter initialization
		if(lastInitAttemptTime > 0) {
			double dt = initEstimate.time - lastInitAttemptTime;
			tNavFilter->totalAttemptTime += dt;
			double dx = double(INCREASE_WINDOW) * (0.01 * 1.5 * dt);
			windowVarInc[0] += dx;
			windowVarInc[2] += dx;
		}
		lastInitAttemptTime = initEstimate.time;

		if(!withinMap) {
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Filter not initialized - vehicle is currently "
				   "within a non-valid region of the reference map\n");
			return;
		}
		if(initEstimate.gpsValid || initEstimate.z <= 1) {
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Filter not initialized - vehicle is currently "
				   "on the surface\n");
			return;
		}
		if(!lastMeasValid) {
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Filter not initialized - vehicle currently "
				   "does not have good range measurements\n");
			return;
		}
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Filter not initialized - vehicle currently "
			   "does not have bottom lock or good velocity data\n");
	}
}

void TerrainNav::initMotion(poseT& initEstimate) {
	tNavFilter->lastNavPose = new poseT;

	//initialize velocity information
	lastValidVel[0] = initEstimate.vx;
	lastValidVel[1] = initEstimate.vy;
	lastValidVel[2] = initEstimate.vz;
	lastVelBotLock = initEstimate.bottomLock;
	tNavFilter->timeLastDvlValid = initEstimate.time;

	//set lastNavEstimate to initEstimate
	*tNavFilter->lastNavPose = initEstimate;

	//add on prior knowledge of navigation offset, if available
	initEstimate += estNavOffset;
	initEstimate.time = tNavFilter->lastNavPose->time;
	initEstimate.dvlValid = tNavFilter->lastNavPose->dvlValid;
	initEstimate.gpsValid = tNavFilter->lastNavPose->gpsValid;
	initEstimate.bottomLock = tNavFilter->lastNavPose->bottomLock;

	//initialize filter with initial pose estimate
	tNavFilter->initFilter(initEstimate);

	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav:: vehicle motion has been initialized\n");

	return;
}



bool TerrainNav::interpolatePoses(const poseT& pose1, const poseT& pose2,
								  poseT& newPose, const double newTime) {
	double deltaT, newDeltaT;

	deltaT = pose2.time - pose1.time;
	newPose.time = newTime;
	newDeltaT = newTime - pose1.time;

	if(newTime > pose2.time || newTime < pose1.time) {
		return false;
	}

	newPose.x = pose1.x + (pose2.x - pose1.x) * newDeltaT / deltaT;
	newPose.y = pose1.y + (pose2.y - pose1.y) * newDeltaT / deltaT;
	newPose.z = pose1.z + (pose2.z - pose1.z) * newDeltaT / deltaT;
	newPose.phi = pose1.phi + (pose2.phi - pose1.phi) * newDeltaT / deltaT;
	newPose.theta = pose1.theta + (pose2.theta - pose1.theta) * newDeltaT / deltaT;
	newPose.psi = pose1.psi + (pose2.psi - pose1.psi) * newDeltaT / deltaT;
	newPose.vx = pose1.vx + (pose2.vx - pose1.vx) * newDeltaT / deltaT;
	newPose.vy = pose1.vy + (pose2.vy - pose1.vy) * newDeltaT / deltaT;
	newPose.vz = pose1.vz + (pose2.vz - pose1.vz) * newDeltaT / deltaT;
	newPose.wx = pose1.wx + (pose2.wx - pose1.wx) * newDeltaT / deltaT;
	newPose.wy = pose1.wy + (pose2.wy - pose1.wy) * newDeltaT / deltaT;
	newPose.wz = pose1.wz + (pose2.wz - pose1.wz) * newDeltaT / deltaT;
	newPose.dvlValid = (pose1.dvlValid && pose2.dvlValid);
	newPose.gpsValid = (pose1.gpsValid && pose2.gpsValid);
	newPose.bottomLock = (pose1.bottomLock && pose2.bottomLock);

	return true;
}

void TerrainNav::computeMeasVariance(measT& currMeas) {
	int sensorIdx = 0;
	double perError;
	double rangeSq;
	int i;

	//find index of current measurement sensor.  If none match,
	//return.
	if(!tNavFilter->findMeasSensorIndex(currMeas.dataType, sensorIdx)) {
		return;
	}
	perError = tNavFilter->vehicle->sensors[sensorIdx].percentRangeError;

	//if covariance vector not already initialized, initialize it
	if(currMeas.covariance == NULL) {
		currMeas.covariance = new double[currMeas.numMeas];
	}

	//compute variance based on sensor's percent range error
	if(currMeas.dataType == TRN_SENSOR_MB) { // mb measurement
		for(i = 0; i < currMeas.numMeas; i++) {
			rangeSq = pow(currMeas.crossTrack[i], 2) + pow(currMeas.alongTrack[i], 2)
					  + pow(currMeas.altitudes[i], 2);
			currMeas.covariance[i] = rangeSq * pow(perError / 100.0, 2);
		}
	} else { //dvl or altimeter measurement
		for(i = 0; i < currMeas.numMeas; i++) {
			currMeas.covariance[i] = pow(currMeas.ranges[i] * perError / 100.0, 2);
		}
	}

	return;
}

void TerrainNav::checkVelocityValidity(poseT& currPose) {
	//check for out of range velocity data - if above max or equal to zero,
	// set dvlValid flag to false
	if((fabs(currPose.vx) > MAX_VEL) || (fabs(currPose.vx) <= 1e-4) ||
			(fabs(currPose.vy) > MAX_VEL) || (fabs(currPose.vz) > MAX_VEL)) {
		currPose.dvlValid = false;
	}

	//check if predicted ground-based acceleration is too large
	//If this is the first velocity measurement, this check won't be performed
	//as lastVelBotLock = false initially;
	if(currPose.bottomLock && lastVelBotLock && currPose.z > 5) {
		if((fabs(currPose.ax) > MAX_ACCEL) || (fabs(currPose.ay) > MAX_ACCEL) ||
				(fabs(currPose.az) > MAX_ACCEL)) {
			currPose.dvlValid = false;
			//update acceleration based on this new information
			currPose.ax = 0;
			currPose.ay = 0;
			currPose.az = 0;
		}
	}
}


void TerrainNav::checkRangeValidity(measT& currMeas) {
	int i, j;
	int numEqual = 0;
	double alpha, dr, dt;
#if MBTRN_DEBUG

	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"measT type:%d\n", currMeas.dataType);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"measT time:%.3f\n", currMeas.time);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"measT #bms:%d\n", currMeas.numMeas);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"measT    x:%.3f\n", currMeas.x);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"measT    y:%.3f\n", currMeas.y);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"measT    z:%.3f\n", currMeas.z);
	logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"measT  hdg:%.3f\n", currMeas.psi);
	for(i = 0; i < currMeas.numMeas; i++)
        {
          if (currMeas.dataType == TRN_SENSOR_MB)
	  logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TRNBeam,%d,%.3f,%d,%.3f,%.3f,%.3f\n",
            i, currMeas.time, currMeas.beamNums[i], currMeas.alongTrack[i],
            currMeas.crossTrack[i], currMeas.altitudes[i]);
          else if (currMeas.dataType == TRN_SENSOR_DVL)
	  logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"DVLBeam,%.3f,%.3f\n",
            i, currMeas.time, currMeas.ranges[i]);

        }
#endif
	//this range check is only valid for DVL and IDT measurements
	if(currMeas.dataType == TRN_SENSOR_DVL) {
		for(i = 0; i < currMeas.numMeas; i++) {
			numEqual = 0;
			alpha = currMeas.ranges[i];
			//check if more than two beams are equal
			if(i < 2) {
				for(j = i + 1; j < currMeas.numMeas; j++) {
					if(fabs(alpha - currMeas.ranges[j]) < 0.1) {
						numEqual++;
					}
				}
				if(numEqual >= 2) {
					//if more than two beams are equal, throw out all beams
					for(j = 0; j < currMeas.numMeas; j++) {
						currMeas.measStatus[j] = false;
					}
					logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav:: Throwing out all beams because more "
					       "than two are equal to %.3f at t=%.2f, ping # %u.\n",
					       alpha, currMeas.time, currMeas.ping_number);
					return;
				}
			}

			//check validity of each beam based on NaN or range value
			if(std::isnan(currMeas.ranges[i]) || (currMeas.ranges[i] >= MAX_RANGE)
					|| (currMeas.ranges[i] <= MIN_RANGE)) {
				currMeas.measStatus[i] = false;
			}

			//check dr/dt for each beam
			if(currMeas.measStatus[i]) {
				if(noValidRange[i]) {
					noValidRange[i] = false;
					lastValidRange[i] = currMeas.ranges[i];
					lastValidRangeTime[i] = currMeas.time;
				} else {
					dr = currMeas.ranges[i] - lastValidRange[i];
					dt = currMeas.time - lastValidRangeTime[i];
					if((dt > 0) && (fabs(dr / dt) > MAX_DRDT)) {
						currMeas.measStatus[i] = false;
					} else {
						lastValidRange[i] = currMeas.ranges[i];
						lastValidRangeTime[i] = currMeas.time;
					}
				}
			}
		}
	}
	else if(currMeas.dataType == TRN_SENSOR_DELTAT){
		for(i = 0; i < currMeas.numMeas; i++) {
			//check validity of each beam based on NaN or range value
		        // Use only the middle 60 of 120 beams
			if(std::isnan(currMeas.ranges[i]) || (currMeas.ranges[i] >= MAX_RANGE)
			   //		|| (currMeas.ranges[i] <= MIN_RANGE) || ((i < 30) || (i >= 90)) )
			   		|| (currMeas.ranges[i] <= MIN_RANGE) || ((i < 30) || (i >  90)) )
			{
			   currMeas.measStatus[i] = false;
			}
			else
			{
			   //currMeas.measStatus[i] = true;
			   //Further pare the center 60 down to only 5
			   //if( i==30 || i==44 || i==59 || i==74 || i==89 )

			   //Try paring down to every 6th for 11 total.
			   if( i%6 == 0 )
			      currMeas.measStatus[i] = true;
			   else
			      currMeas.measStatus[i] = false;
			   //
			   // measStatus is set back to false during trip from MVC?
			}
		}
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::measUpdate() - IDT[45] = %.2f, IDT[75] = %.2f\n",
			currMeas.ranges[45], currMeas.ranges[75]);
	}
	else {
		return;
	}

}

//void TerrainNav::reinitFilter(int newState, bool lowInfoTransition) {
//old state machine declaratino: void TerrainNav::reinitFilter(int newState, bool lowInfoTransition) {
void TerrainNav::reinitFilter(bool lowInfoTransition) {
	int interpMapMethod = 1;
	bool interpMeasAttitude = true;
	double driftRate = 1;
	unsigned int distrib_type = 0;
	int useModWeight = useModifiedWeighting;

	//Default initialization window --> mainly used for broad area reinitializations
	double windowVar[N_COVAR] = {
			 X_STDDEV_INIT * X_STDDEV_INIT,
			 0.0, Y_STDDEV_INIT * Y_STDDEV_INIT,
			 0.0, 0.0, Z_STDDEV_INIT * Z_STDDEV_INIT,
			 0.0, 0.0, 0.0, PHI_STDDEV_INIT * PHI_STDDEV_INIT,
			 0.0, 0.0, 0.0, 0.0, THETA_STDDEV_INIT * THETA_STDDEV_INIT,
			 0.0, 0.0, 0.0, 0.0, 0.0, PSI_STDDEV_INIT * PSI_STDDEV_INIT,
			 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, GYRO_BIAS_STDDEV_INIT * GYRO_BIAS_STDDEV_INIT,
			 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, GYRO_BIAS_STDDEV_INIT * GYRO_BIAS_STDDEV_INIT,
			 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, PSI_BERG_STDDEV_INIT * PSI_BERG_STDDEV_INIT
						   };
	int i;
	poseT* temp = new poseT;

	//ensure that tNavFilter is non-empty before accessing and deleting
	if(tNavFilter != NULL) {
		//copy relevant data from current filter
		interpMapMethod = tNavFilter->GetInterpMapMethod();
		interpMeasAttitude = tNavFilter->interpMeasAttitude;
		driftRate = tNavFilter->vehicle->driftRate;
		distrib_type = tNavFilter->getDistribToSave();
                logs(TL_OMASK(TL_TERRAIN_NAV, TL_BOTH), "reinitFilter: getDistribToSave == %d\n", distrib_type);

		// More than one setting is encoded in the useModifiedSetting value,
		// so always use the one received from the client through the setModifiedWeighting() call
		//useModWeight = tNavFilter->useModifiedWeighting;  // RGH: Use the value received from the client

		//If keeping mean and covariance
		if(tNavFilter->lastNavPose != NULL && !lowInfoTransition) {
			tNavFilter->computeMMSE(temp);	//puts covariance and MMSE in temp
			for(i = 0; i < N_COVAR; i++) {
				windowVar[i] = 1.0 * temp->covariance[i];
			}
		}

		//delete filter object
		delete tNavFilter;
		tNavFilter = NULL;
	}

	/*//old state machine version:
	//create new filter based on the newState
	switch(newState) {
		case 0:
			//Create new Sigma Point filter (NOMINALLY, CURRENTLY USING PARTICLE FILTER (state 2))
			//Create new Sigma Point filter (NOMINALLY, CURRENTLY USING PARTICLE FILTER (filter type 2))
			createFilter(2, windowVar);
			break;

		case 1:
			//Create new Particle filter
			createFilter(2, windowVar);
			break;

		case 2:
			//If low information transition, reinitialize with uniform distribution
			//and initial search window (larger search window because using PMF)
			if(lowInfoTransition) {
				for(i = 0; i < N_COVAR; i++) {
					windowVar[i] *= 1.0;  // want to use smaller value if starting w/ init window, but bigger if taking current var
				}
			}
			if(this->mapType == 2) {
				//Create Particle Filter from windowVar  TODO: don't only use PF with octree, but allow PMF too
				createFilter(2, windowVar);
			} else {
				//Create Point Mass Filter from windowVar  TODO: fix PMF so that it can be used here for both cases
				createFilter(2, windowVar);
			}
			tNavFilter->setInitDistribType(0); //0 is uniform, 1 is gaussian
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::reinitializing filter from lowInfoTransition with uniform distribution \n");
			break;

		default:
			createFilter(2, windowVar);
	}
	filterState = newState;
	*/

	createFilter(this->filterType, windowVar);



	//if not transitioning due to low information, initialize the filter
	//with a Gaussian distribution
	if(!lowInfoTransition) {
		tNavFilter->setInitDistribType(1);
	}



	//reset filter and terrainNav parameters
	setMapInterpMethod(interpMapMethod);
	setVehicleDriftRate(driftRate);
	setInterpMeasAttitude(interpMeasAttitude);
	setModifiedWeighting(useModWeight);
	if (tNavFilter != NULL) tNavFilter->setDistribToSave(distrib_type);

	numWaitingMeas = 0;
	lastMeasSuccessTime = -1.0;
	lastInitAttemptTime = -1.0;
	lastBottomLockTime = -1.0;
	numReinits++;
	delete temp;
}

bool TerrainNav::checkFilterHealth() {
	bool healthy = true; //1 is healthy, 0 is not healthy and
	//needs to be reinitialized
	bool lowInfoTransition = false;

	//double currVarArea;
	//currVarArea = (tNavFilter->currVar[0] + tNavFilter->currVar[1]);

	//double larger;
	//larger = max(tNavFilter->currVar[0], tNavFilter->currVar[1]);

	//initialize the state to change to as the current state
	//int newState = filterState;
	//old state machine version: int newState = filterState;

	//check if the length of time since last successful measurement exceeds
	//set maximum. Ensure that the filter has been initialized and that there
	//has been at least one successful measurement incorporated
	if(this->lastMeasSuccessTime > 0 && tNavFilter->lastNavPose != NULL
			&& tNavFilter->lastNavPose->time - this->lastMeasSuccessTime > MAX_MEAS_OUTAGE) {
		//old state machine version: newState = transitionMatrix[3][filterState];
		healthy = false;
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::No valid range measurements for the past %.1f "
			   "seconds. Re-initializing the filter.\n",  MAX_MEAS_OUTAGE);
		lowInfoTransition = true;

	}


	//check if the length of time since last successful bottom velocity meas
	//exceeds set maximum. Ensure that the filter has been initialized and that
	//there has been at least one successful measurement incorporated
	if(this->lastBottomLockTime > 0 && tNavFilter->lastNavPose != NULL
			&& tNavFilter->lastNavPose->time - this->lastBottomLockTime > MAX_VEL_OUTAGE) {
		//old state machine version: newState = transitionMatrix[3][filterState];
		healthy = false;
		lowInfoTransition = true;
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::No valid bottom velocity measurements for the past %.1f "
			   "seconds.  Re-initializing the filter.\n",
			   MAX_VEL_OUTAGE);
	}
	/*//old state machine version:
	//check if x/y uncertainty of the filter is below a set minimum.
	//potentially for switching filters in the future.
	// force reinit is for testing the filter reinitialization only
	if(healthy && currVarArea < MIN_FILTER_VAR)
		//if(healthy && larger < MIN_FILTER_VAR)
	{
		newState = transitionMatrix[0][filterState];
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"CHECK TerrainNav:: TransitionMatrix is [%i %i %i]", transitionMatrix[0][0], transitionMatrix[0][1],
			   transitionMatrix[0][2]);
		if(newState != filterState) {
			healthy = false;
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::North/East uncertainty has fallen below the "
				   "minimum of %.1f m^2.  Re-initializing the filter.\n",
				   MIN_FILTER_VAR);
		}
	}*/

	//check if measurement variance is below a set minimum
	/*if(tNavFilter->measVariance > 0 && tNavFilter->measVariance < MIN_MEAS_VAR)
	{
			healthy = false;
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::Measurement variance has fallen below the minimum "
		     "of %f .  Re-initializing the filter.\n", MIN_MEAS_VAR);
	}
	*/

	/*//old state machine version:
	//check if the x/y uncertainty of the filter exceeds a set maximum
	if(healthy && currVarArea > MAX_FILTER_VAR)
		//if(healthy && larger > MAX_FILTER_VAR)
	{
		newState = transitionMatrix[2][filterState];
		if(newState != filterState) {
			healthy = false;
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::North/East uncertainty has exceeded the "
				   "maximum of %.1f m^2.  Re-initializing the filter.\n",
				   MAX_FILTER_VAR);
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"North: %f East: %f Comb: %f. \n", tNavFilter->currVar[0],
				   tNavFilter->currVar[1],	currVarArea);
		}
	}*/

	/*//old state machine version:
	//check if the uncertainty of the filter has changed enough to warrant
	//changing states
	if(healthy && currVarArea < MAX_FILTER_VAR - 1 * VAR_MARGIN &&
			currVarArea > MIN_FILTER_VAR + 0.1 * VAR_MARGIN)*/
		/*if(healthy && larger < MAX_FILTER_VAR-1*VAR_MARGIN &&
				larger > MIN_FILTER_VAR+0.1*VAR_MARGIN)*/
	/*//old state machine version:
	{
		newState = transitionMatrix[1][filterState];
		if(newState != filterState) {
			healthy = false;
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::North/East uncertainty is within the margins "
				   "of %.1f m^2.  Re-initializing the filter.\n",
				   VAR_MARGIN);
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"North: %f East: %f Comb: %f. \n", tNavFilter->currVar[0],
				   tNavFilter->currVar[1], currVarArea);
		}
	}*/

	//check the windowed average of the Normalized Innovations Squared
	if(healthy && tNavFilter->windowedNIS > MAX_NIS_VALUE) {
		//***TODO Figure out how this should work
		//old state machine version: newState = transitionMatrix[1][filterState];
		healthy = false;
		lowInfoTransition = true;
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav:: Windowed NIS average %.2f exceeds the maximum allowed "
			   "of %.1f.  Re-initializing the filter.\n", tNavFilter->windowedNIS, MAX_NIS_VALUE);
	} else {
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav:: Windowed NIS is %.1f, in allowable region under "
			   "%.1f.  Keep on Trucking.\n", tNavFilter->windowedNIS, MAX_NIS_VALUE);
	}


	if(healthy && tNavFilter->SubcloudNIS > MAX_NIS_VALUE){
		healthy = false;
		lowInfoTransition = true;
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),
			"TerrainNav:: SubcloudNIS of %.2f exceeds the maximum allowed of %.2f. "
			"Re-initializing the filter.\n", tNavFilter->SubcloudNIS, MAX_NIS_VALUE);
	} else {
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),
			"TerrainNav:: SubcloudNIS is %.2f, in allowable region under "
			"%.2f.  Keep on Trucking.\n", tNavFilter->SubcloudNIS, MAX_NIS_VALUE);
	}

	//if filter is not healthy reinitialize
	if(!healthy) {
		//old state machine version: reinitFilter(newState, lowInfoTransition);
		reinitFilter(lowInfoTransition);
	}

	return healthy;

}

/* Function: copyToLogDir() - copies configuration files to the log directory
*			     for future reference
*  A log directory (saveDirectory) must be specified in the initialization
*  message, otherwise nothing is copied.
*/
void TerrainNav::copyToLogDir()
{
	// Used the following line for testing
	// this->saveDirectory = "./logdir";

	// Copy only if there is a place for the files to land
	//
	char *slash = NULL;
	if (NULL != this->saveDirectory)
	{
		char* trnLogDir = getenv("TRN_LOGFILES");
                char dot[]=".";
		if (!trnLogDir) trnLogDir = dot;
		char dir_spec[500];  // Reusable buffer to write log directory paths

		// Create the log directory
		//
		if( ( slash = strrchr( this->saveDirectory, '/' ) ) )
		{
		   *slash = '\0';
		}
		sprintf(dir_spec, "%s/%s-TRN", trnLogDir, this->saveDirectory);

		for (int n = 1; 0 != mkdir(dir_spec, 0777); n++)
		{
       if ( errno != EEXIST )
       {
          // If the error is about something other than EEXIST
          //
          fprintf(stderr,"\n\n\t\tTerrainNav::copyToLogDir() - ERR: mkdir(%s) failed"
          " [%d,%s]\n\n", dir_spec, errno, strerror(errno));
          trnLogDir = dot;
	  break;
       }
       else sprintf(dir_spec, "%s/%s-TRN.%02d", trnLogDir, this->saveDirectory, n);
		}

		free(this->saveDirectory);
		this->saveDirectory = STRDUPNULL(dir_spec);
		logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG), "TRN log directory is %s\n", this->saveDirectory);


		// Create a latest link that points to the log directory
		//
		sprintf(dir_spec, "%s/%s", trnLogDir, LatestLogDirName);
		remove(dir_spec);
		if (0 != symlink(this->saveDirectory, dir_spec))
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG), "symlink %s to %s failed:%s\n",
				dir_spec, this->saveDirectory, strerror(errno));
		else
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG), "symlink %s to %s OK\n",
				dir_spec, this->saveDirectory);
	}
	else
		return;

	char copybuf[320];
	if (NULL != this->vehicleSpecFile)
	{
		sprintf(copybuf, "cp %s %s/.", this->vehicleSpecFile,
						    this->saveDirectory);
		if (0 != system(copybuf))
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG), "command \'%s\' failed:%s\n",
				copybuf, strerror(errno));
	}

	if (NULL != this->particlesFile)
	{
		sprintf(copybuf, "cp %s %s/.", this->particlesFile,
						    this->saveDirectory);
		if (0 != system(copybuf))
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG), "command \'%s\' failed:%s\n",
				copybuf, strerror(errno));
	}

	// Create a vehicleT object just to get the sensor spec files to copy
	//
	vehicleT *v = new vehicleT(vehicleSpecFile);
	for (int i = 0; i < v->numSensors; i++)
	{
		sprintf(copybuf, "cp %s %s/.", v->sensors[i].filename,
						       this->saveDirectory);
		if (0 != system(copybuf))
			logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG), "command \'%s\' failed:%s\n",
				copybuf, strerror(errno));
	}
	delete v;
}
