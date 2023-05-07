#include <iostream>
#include <fstream>
#include <cstdlib>
#include "pin.H"
// My libraries
#include <map>
//
using std::cerr;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;

// My namespaces
using std::map;
using std::nothrow;
// Simulation will stop when this number of instructions have been executed
//
#define STOP_INSTR_NUM 1000000000 // 1b instrs

// Simulator heartbeat rate
//
#define SIMULATOR_HEARTBEAT_INSTR_NUM 100000000 // 100m instrs

/* Base branch predictor class */
// You are highly recommended to follow this design when implementing your branch predictors
//
class BranchPredictorInterface {
public:
  //This function returns a prediction for a branch instruction with address branchPC
  virtual bool getPrediction(ADDRINT branchPC) = 0;

  //This function updates branch predictor's history with outcome of branch instruction with address branchPC
  virtual void train(ADDRINT branchPC, bool branchWasTaken) = 0;
};

// This is a class which implements always taken branch predictor
class AlwaysTakenBranchPredictor : public BranchPredictorInterface {
public:
  AlwaysTakenBranchPredictor(UINT64 numberOfEntries) {}; //no entries here: always taken branch predictor is the simplest predictor
	virtual bool getPrediction(ADDRINT branchPC) {
		return true; // predict taken
	}
	virtual void train(ADDRINT branchPC, bool branchWasTaken) {} //nothing to do here: always taken branch predictor does not have history
};

//------------------------------------------------------------------------------
//##############################################################################
/*
 * Insert your changes below here...
 *
 * Put your branch predictor implementation here
 *
 * For example:
 * class LocalBranchPredictor : public BranchPredictorInterface {
 *
 *   ***put private members for Local branch predictor here
 *
 *   public:
 *	   virtual bool getPrediction(ADDRINT branchPC) {
 *	  	 ***put your implementation here
 *	   }
 *	   virtual void train(ADDRINT branchPC, bool branchWasTaken) {
 *	     ***put your implementation here
 *	   }
 * }
 *
 * You also need to create an object of branch predictor class in main()
 * (i.e. at line 193 in the original unmodified version of this file).
 */
//##############################################################################
//------------------------------------------------------------------------------

// Local
class LocalBranchPredictor:
    public BranchPredictorInterface{
      // Local History Registers
      UINT64 LHR [128];
      // Dynamically allocated Pattern History Table
      UINT64 * PHT;
      // The number of entries in the PHT
      int modul;
      public:
      // This constructor sets up the Branch predictor
      LocalBranchPredictor(UINT64 numberOfEntries) {
          // Called it modul since it takes the modulus of the PC
          modul = numberOfEntries;
          // Alocating the PHT
          PHT = new (nothrow) UINT64[modul];
          for (int i = 0; i < modul; i++)
          { // Initialise the LHT until 128
            if(i<128)
            {
             LHR[i] = 0;
            }
            // Initilise the PHT
            PHT[i] = 3;
          }
      };
      // This function returns the prediction
      virtual bool getPrediction(ADDRINT branchPC){
            // Get the Index from PC
            UINT64 index = (branchPC)%128;
            // Index the LHR with the index which is used to index the PHT, >>1 makes it shift to the predicting bit
            bool prediction = (bool) (PHT[LHR[index]]>>1);
            return prediction;
      }
      //This function updates branch predictor's history with outcome of branch instruction with address branchPC
      virtual void train(ADDRINT branchPC,bool branchWasTaken){
            // Get the Index from PC
             UINT64 index = (branchPC)%128;
             // index of the LHR
             UINT64 i = LHR[index];
             bool PHT_prediction = (bool)(PHT[i]>>1);

             if(branchWasTaken){
               // if Branch taken -> PHT taken -> increment (if not strongly taken)
               if(PHT_prediction){
                 if(PHT[i]!=3){
                   PHT[i] = PHT[i]+1;
                 }
               }
               // if Branch taken -> PHT not taken -> increment (misprediction hence increment to fix)
               else
               {
                 PHT[i] = PHT[i]+1;}

             }
             else
             { // if Branch not taken -> PHT taken -> decrement (misprediction hence decrement to fix)
               if(PHT_prediction){
                 PHT[i] = PHT[i]-1;

               }
               // if Branch not taken -> PHT not taken -> decrement (if not weakly not taken)
               else
               { if(PHT[i]>0){
                 PHT[i] = PHT[i]-1; }
               }
             }
             // Update the LHR
             LHR[index] = ( (LHR[index]<<1)+ branchWasTaken)%modul;
      }
    };

// Gshare
class GshareBranchPredictor:
   public BranchPredictorInterface {
     // Global History Register
     UINT64 GHR;
     // Dynamically allocated Pattern History Table
     UINT64 * PHT;
     // The number of entries in the PHT
     int modul;
public:
// This constructor sets up the Branch predictor
  GshareBranchPredictor(UINT64 numberOfEntries) {
          // Initialise Modulus,PHT and GHT
          modul = numberOfEntries;
          PHT = new (nothrow) UINT64[modul];
          GHR = 0;
          for (int i = 0; i < modul; i++)
          {
            PHT[i] = 3;
          }
  };
  // This function returns the prediction
	virtual bool getPrediction(ADDRINT branchPC) {
    // index is different from Local, now its (PC mod number_of_PHT_entries)
     UINT64 index = (branchPC)%modul;
     index = index ^ GHR;
     bool prediction = (bool) (PHT[index]>>1);
     return prediction;

	}
  //This function updates branch predictor's history with outcome of branch instruction with address branchPC
	virtual void train(ADDRINT branchPC, bool branchWasTaken) {
    // calculate index
    UINT64 i = (branchPC)%modul;
    // xor with GHR to index into the PHT
    i = i ^ GHR;
    bool PHT_prediction = (bool)(PHT[i]>>1);
    if(branchWasTaken){
               // if Branch taken -> PHT taken -> increment (if not strongly taken)
               if(PHT_prediction){
                 if(PHT[i]!=3){
                 PHT[i] = PHT[i]+1;
                 }
               }
              // if Branch taken -> PHT not taken -> increment (misprediction hence increment to fix)
               else
               {
                 PHT[i] = PHT[i]+1;}

             }
     else
             { // if Branch not taken -> PHT taken -> decrement (misprediction hence decrement to fix)
               if(PHT_prediction){
                 PHT[i] = PHT[i]-1;

               }
               // if Branch not taken -> PHT not taken -> decrement (if not weakly not taken)
               else
               { if(PHT[i]>0){
                 PHT[i] = PHT[i]-1; }
               }
             }
     // GHR is updated to the actual outcome
     GHR = ((GHR<<1)+branchWasTaken)%modul;
  }
};

// Tournament
class TournamentBranchPredictor:
public BranchPredictorInterface {
  // Pattern History Table and modulus
  UINT64 * PHT;
  int modul;
  // Intialise the class pointer for Local and Global
  LocalBranchPredictor * Local;
  GshareBranchPredictor * Global;

public:
  TournamentBranchPredictor(UINT64 numberOfEntries) {
          // Initialise PHT, modulus and our classes
          modul = numberOfEntries;
          PHT = new (nothrow) UINT64[modul];
          for (int i = 0; i < modul; i++)
          {
            PHT[i] = 3;
          }
          Local = new LocalBranchPredictor(numberOfEntries);
          Global = new GshareBranchPredictor(numberOfEntries);

  };
	virtual bool getPrediction(ADDRINT branchPC) {
    // Caculate the index like global, it is calculated differently
    UINT64 index = (branchPC)%modul;
    bool prediction;
    // Depending on the Value of the PHT, get prediction from the appropriate class
    if((bool) (PHT[index]>>1)) {
     prediction=Global->getPrediction(branchPC);
     }
    else{
     prediction=Local->getPrediction(branchPC);
     }

		return prediction;
	}
	virtual void train(ADDRINT branchPC, bool branchWasTaken) {
    // Calculate the index
    UINT64 i = (branchPC)%modul;
    // all the predictions from all the branches
    bool gshare_pred = Global->getPrediction(branchPC);
    bool local_pred = Local->getPrediction(branchPC);
    bool pick_class = (bool) (PHT[i]>>1);
    // Global
    if(pick_class){
      // IF Taken -> Gshare is taken -> increment
      if(branchWasTaken){
        if(gshare_pred){
          if(PHT[i]!=3){
            PHT[i] = PHT[i]+1;
                 }
               }
          // if Taken -> Gshare is not taken -> if Local is taken -> decrement
          else
          {
            if(local_pred) {
                PHT[i] = PHT[i]-1;}

             } }
      else{
          //if not taken -> Gshare is taken -> if local is not taken -> decrement
          if(gshare_pred&&!local_pred){
            PHT[i] = PHT[i]-1;
          }
          // if not taken -> Gshare is not taken -> decrement
          else{
            if(PHT[i]!=3){
              PHT[i] = PHT[i]+1;
          }

            }
            }
    }
      // Local 0,1
      else
          {
             if(branchWasTaken){
               // if taken -> local taken -> decrement
               if(local_pred){
                 if(PHT[i]>0){
                 PHT[i] = PHT[i]-1;}

               }
               // if taken -> local not taken -> gshare is taken -> increment
               else
               { if(gshare_pred) {
                 PHT[i] = PHT[i]+1;
                 }
                  }
             }
             else{
                // if not taken -> local is taken -> ghsare is not taken -> increment
                if(local_pred&&!gshare_pred){
                  PHT[i] = PHT[i]+1;

               }
               // if not taken -> gshare is not taken -> decrement
               else
               {
                 if(PHT[i]>0){
                   PHT[i] = PHT[i]-1;
                 }
             }
             }

    }

Local->train(branchPC,branchWasTaken);
Global->train(branchPC,branchWasTaken);
}

};



ofstream OutFile;
BranchPredictorInterface *branchPredictor;

// Define the command line arguments that Pin should accept for this tool
//
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "BP_stats.out", "specify output file name");
KNOB<UINT64> KnobNumberOfEntriesInBranchPredictor(KNOB_MODE_WRITEONCE, "pintool",
    "num_BP_entries", "1024", "specify number of entries in a branch predictor");
KNOB<string> KnobBranchPredictorType(KNOB_MODE_WRITEONCE, "pintool",
    "BP_type", "always_taken", "specify type of branch predictor to be used");

// The running counts of branches, predictions and instructions are kept here
//
static UINT64 iCount                          = 0;
static UINT64 correctPredictionCount          = 0;
static UINT64 conditionalBranchesCount        = 0;
static UINT64 takenBranchesCount              = 0;
static UINT64 notTakenBranchesCount           = 0;
static UINT64 predictedTakenBranchesCount     = 0;
static UINT64 predictedNotTakenBranchesCount  = 0;

VOID docount() {
  // Update instruction counter
  iCount++;
  // Print this message every SIMULATOR_HEARTBEAT_INSTR_NUM executed
  if (iCount % SIMULATOR_HEARTBEAT_INSTR_NUM == 0) {
    std::cerr << "Executed " << iCount << " instructions." << endl;
  }
  // Release control of application if STOP_INSTR_NUM instructions have been executed
  if (iCount == STOP_INSTR_NUM) {
    PIN_Detach();
  }
}



VOID TerminateSimulationHandler(VOID *v) {
  OutFile.setf(ios::showbase);
  // At the end of a simulation, print counters to a file
  OutFile << "Prediction accuracy:\t"            << (double)correctPredictionCount / (double)conditionalBranchesCount << endl
          << "Number of conditional branches:\t" << conditionalBranchesCount                                      << endl
          << "Number of correct predictions:\t"  << correctPredictionCount                                        << endl
          << "Number of taken branches:\t"       << takenBranchesCount                                            << endl
          << "Number of non-taken branches:\t"   << notTakenBranchesCount                                         << endl
          ;
  OutFile.close();

  std::cerr << endl << "PIN has been detached at iCount = " << STOP_INSTR_NUM << endl;
  std::cerr << endl << "Simulation has reached its target point. Terminate simulation." << endl;
  std::cerr << "Prediction accuracy:\t" << (double)correctPredictionCount / (double)conditionalBranchesCount << endl;
  std::exit(EXIT_SUCCESS);
}

//
VOID Fini(int code, VOID * v)
{
  TerminateSimulationHandler(v);
}

// This function is called before every conditional branch is executed
//
static VOID AtConditionalBranch(ADDRINT branchPC, BOOL branchWasTaken) {
  /*
	 * This is the place where the predictor is queried for a prediction and trained
	 */

  // Step 1: make a prediction for the current branch PC
  //
	bool wasPredictedTaken = branchPredictor->getPrediction(branchPC);

  // Step 2: train the predictor by passing it the actual branch outcome
  //
	branchPredictor->train(branchPC, branchWasTaken);

  // Count the number of conditional branches executed
  conditionalBranchesCount++;

  // Count the number of conditional branches predicted taken and not-taken
  if (wasPredictedTaken) {
    predictedTakenBranchesCount++;
  } else {
    predictedNotTakenBranchesCount++;
  }

  // Count the number of conditional branches actually taken and not-taken
  if (branchWasTaken) {
    takenBranchesCount++;
  } else {
    notTakenBranchesCount++;
  }

  // Count the number of correct predictions
	if (wasPredictedTaken == branchWasTaken)
    correctPredictionCount++;
}

// Pin calls this function every time a new instruction is encountered
// Its purpose is to instrument the benchmark binary so that when
// instructions are executed there is a callback to count the number of
// executed instructions, and a callback for every conditional branch
// instruction that calls our branch prediction simulator (with the PC
// value and the branch outcome).
//
VOID Instruction(INS ins, VOID *v) {
  // Insert a call before every instruction that simply counts instructions executed
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);

  // Insert a call before every conditional branch
  if ( INS_IsBranch(ins) && INS_HasFallThrough(ins) ) {
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AtConditionalBranch, IARG_INST_PTR, IARG_BRANCH_TAKEN, IARG_END);
  }
}

// Print Help Message
INT32 Usage() {
  cerr << "This tool simulates different types of branch predictors" << endl;
  cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
  return -1;
}

int main(int argc, char * argv[]) {
  // Initialize pin
  if (PIN_Init(argc, argv)) return Usage();

  // Create a branch predictor object of requested type
  if (KnobBranchPredictorType.Value() == "always_taken") {
    std::cerr << "Using always taken BP" << std::endl;
    branchPredictor = new AlwaysTakenBranchPredictor(KnobNumberOfEntriesInBranchPredictor.Value());
  }
//------------------------------------------------------------------------------
//##############################################################################
/*
 * Insert your changes below here...
 *
 * In the following cascading if-statements instantiate branch predictor objects
 * using the classes that you have implemented for each of the three types of
 * predictor.
 *
 * The choice of predictor, and the number of entries in its prediction table
 * can be obtained from the command line arguments of this Pin tool using:
 *
 *  KnobNumberOfEntriesInBranchPredictor.Value()
 *    returns the integer value specified by tool option "-num_BP_entries".
 *
 *  KnobBranchPredictorType.Value()
 *    returns the value specified by tool option "-BP_type".
 *    The argument of tool option "-BP_type" must be one of the strings:
 *        "always_taken",  "local",  "gshare",  "tournament"
 *
 *  Please DO NOT CHANGE these strings - they will be used for testing your code
 */
//##############################################################################
//------------------------------------------------------------------------------
  else if (KnobBranchPredictorType.Value() == "local") {
  	 std::cerr << "Using Local BP." << std::endl;
/* Uncomment when you have implemented a Local branch predictor */
    branchPredictor = new LocalBranchPredictor(KnobNumberOfEntriesInBranchPredictor.Value());
  }
  else if (KnobBranchPredictorType.Value() == "gshare") {
  	 std::cerr << "Using Gshare BP."<< std::endl;
/* Uncomment when you have implemented a Gshare branch predictor */
    branchPredictor = new GshareBranchPredictor(KnobNumberOfEntriesInBranchPredictor.Value());
  }
  else if (KnobBranchPredictorType.Value() == "tournament") {
  	 std::cerr << "Using Tournament BP." << std::endl;
/* Uncomment when you have implemented a Tournament branch predictor */
    branchPredictor = new TournamentBranchPredictor(KnobNumberOfEntriesInBranchPredictor.Value());
  }
  else {
    std::cerr << "Error: No such type of branch predictor. Simulation will be terminated." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  std::cerr << "The simulation will run " << STOP_INSTR_NUM << " instructions." << std::endl;

  OutFile.open(KnobOutputFile.Value().c_str());

  // Pin calls Instruction() when encountering each new instruction executed
  INS_AddInstrumentFunction(Instruction, 0);

  // Function to be called if the program finishes before it completes 10b instructions
  PIN_AddFiniFunction(Fini, 0);

  // Callback functions to invoke before Pin releases control of the application
  PIN_AddDetachFunction(TerminateSimulationHandler, 0);

  // Start the benchmark program. This call never returns...
  PIN_StartProgram();

  return 0;
}
