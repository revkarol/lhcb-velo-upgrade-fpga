#include "GaudiKernel/AlgFactory.h"

#include "Kernel/VPChannelID.h"

#include "VPSuperPixelBankEncoder.h"
#include "Event/ODIN.h"


using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : VPSuperPixelBankEncoder
//
// 2014-03-06 : Karol Hennessy, Kurt Rinnert
//-----------------------------------------------------------------------------

DECLARE_ALGORITHM_FACTORY(VPSuperPixelBankEncoder)

	//=============================================================================
	// Constructor
	//=============================================================================
VPSuperPixelBankEncoder::VPSuperPixelBankEncoder(const std::string& name,
		ISvcLocator* pSvcLocator)
: GaudiAlgorithm(name, pSvcLocator),
	m_bankVersion(2),
	m_isDebug(false),
	m_isVerbose(false),
        //m_evt(5749),  // hack to sum mu and md files
	m_evt(0),  // hack to sum mu and md files
	m_spBySensor(208, std::vector<unsigned int>()) ,
        numHits(0),
        numSPs(0)
        {
		declareProperty("DigitLocation",
				m_digitLocation = LHCb::VPDigitLocation::Default);
		declareProperty("RawEventLocation",
				m_rawEventLocation = LHCb::RawEventLocation::Default);
	}

//=============================================================================
// Destructor
//=============================================================================
VPSuperPixelBankEncoder::~VPSuperPixelBankEncoder() { ; }

//=============================================================================
// Initialization
//=============================================================================
StatusCode VPSuperPixelBankEncoder::initialize() {
	StatusCode sc = GaudiAlgorithm::initialize();
	if (sc.isFailure()) return sc;
	m_isDebug = msgLevel(MSG::DEBUG);
	m_isVerbose = msgLevel(MSG::VERBOSE);
	if (m_isDebug) debug() << "==> Initialise" << endmsg;
	f = fopen("test.bin", "wb");
        histfile = new TFile("boolehists_karol.root", "recreate");
        h_hits = new TH2F("h_hits", "hits vs chip", 624, -0.5, 623.5, 1000, -0.5, 999.5);
        h_nsps = new TH2F("h_nsps", "SPs vs chip", 624, -0.5, 623.5, 1000, -0.5, 999.5);
	return StatusCode::SUCCESS;
}

//=============================================================================
//  Execution
//=============================================================================
StatusCode VPSuperPixelBankEncoder::execute() {

	if (m_isDebug) debug() << "==> Execute" << endmsg;
	++m_evt;
	const VPDigits* digits = getIfExists<VPDigits>(m_digitLocation);
	if (NULL == digits) {
		return Error(" ==> There are no VPDigits in TES! ");
	}

   // Load the ODIN
      const LHCb::ODIN* odin = getIfExists<ODIN>(evtSvc(),LHCb::ODINLocation::Default);
      if ( !odin ) { odin = getIfExists<ODIN>(evtSvc(),LHCb::ODINLocation::Default,false); }
      if ( !odin )
      {
        // should always be available ...
        return Error( "Cannot load the ODIN data object", StatusCode::SUCCESS );
      }
	unsigned long long oevt = odin->eventNumber();
	unsigned int bunch = odin->bunchId();
	printf("evt %d odin event %ld bunch %d\n", m_evt, oevt, bunch);
	// Check if RawEvent exists
	RawEvent* rawEvent = getIfExists<RawEvent>(m_rawEventLocation);
	if (NULL == rawEvent) {
		// Create RawEvent
		rawEvent = new LHCb::RawEvent();
		put(rawEvent, m_rawEventLocation);
	}

	for (unsigned int sensor = 0; sensor < m_spBySensor.size(); ++sensor) {
		m_spBySensor[sensor].clear();
		// put header word as first element (but don't know content yet)
		m_spBySensor[sensor].push_back(0);
	}

	std::vector< std::vector<unsigned int> > spByChip(12*52);
	for(unsigned int i=0; i<spByChip.size(); i++){
		// put size as first element (but don't know yet)
		spByChip[i].clear();
		spByChip[i].push_back(0);
	}

	// Loop over digits create super pixel words and store them.
	// No assumption about the order of digits is made.
	LHCb::VPDigits::const_iterator itSeed;
        
	int evtHits = 0;
        std::vector<int> hits_per_chip(624,0);
        std::vector<int> nsps_per_chip(624,0);
	for (itSeed = digits->begin(); itSeed != digits->end(); ++itSeed) {
		const unsigned int chip = (*itSeed)->channelID().chip();
		const unsigned int mod = (*itSeed)->channelID().module();//0-51
		const unsigned int row = (*itSeed)->channelID().row();
		const unsigned int col = (*itSeed)->channelID().col();
		const unsigned int sensor = (*itSeed)->channelID().sensor();
                const unsigned int globalChip = chip+ (sensor*3);
		const unsigned int sensorCol = col + 256 * chip;
		const unsigned int chipCol = col;
		const unsigned int spCol = chipCol / 2;
		const unsigned int spColSensor = sensorCol / 2;
		const unsigned int spRow = row / 4;
		const unsigned int spAddr = ((spCol << 6) | spRow);
		const unsigned int spAddrSensor = ((spColSensor << 6) | spRow);
		const unsigned int pix = 1 << (((col % 2) * 4) + row % 4);
                const unsigned int spix = (pix&0xff) | (( spAddr << 8 ) & 0x1fff00) ;

		bool foundS = false;
		bool foundC = false;
		//debug() << "chip "<< chip << " globalChip " << globalChip << " sensor " << sensor << " mod " << mod << endmsg;
                if (globalChip == 111) 
                        printf("chip=%d, mod=%d, globalChip=%d, evt=%d, row=%d, col=%d\n", chip, mod, globalChip, m_evt, row, col);

		for (unsigned int j = 0; j < m_spBySensor[sensor].size(); j++) {
			if ((m_spBySensor[sensor][j] >> 8) == spAddrSensor) {
				m_spBySensor[sensor][j] |= pix;
				foundS = true;
				break;
			}
		}


		for(unsigned int j=0; j<spByChip[globalChip].size(); j++){
			if((spByChip[globalChip][j] >> 8) == spAddr ){
				spByChip[globalChip][j] |= spix;
				foundC = true;
				break;
			}
		}

		if (!foundC){
			spByChip[globalChip].push_back(spix);
			numSPs++;
		}

		if (!foundS) {
			m_spBySensor[sensor].push_back((spAddr << 8) | pix);
		}


                hits_per_chip[globalChip]++;
		numHits++;
		evtHits++;
	}

        for (int i=0; i<624; ++i){
                
                h_hits->Fill(i, hits_per_chip[i]);
                hits_per_chip[i] = 0;
        }

	info() << "Hits this event " << evtHits<< endmsg;

	// randomise
	for(unsigned int i=0; i<spByChip.size(); i++){
		// put size as first element (but don't know yet)
		std::random_shuffle(spByChip[i].begin()+1, spByChip[i].end());
		//spByChip[i].insert(0,0);
	}

	// set 'no neighbour' hint flags
	int dx[] = { -1, 0, 1, -1, 0, 1, -1, 1 };
	int dy[] = { -1, -1, -1, 1, 1, 1, 0, 0 };

	for (unsigned int sens = 0; sens < m_spBySensor.size(); ++sens) {

		if (m_spBySensor[sens].size() < 2) {
			continue;
		}  // empty bank

		// sort super pixels column major on each sensor
		std::sort(m_spBySensor[sens].begin() + 1, m_spBySensor[sens].end(),
				SPLowerThan());

		memset(m_buffer, 0, 24576 * sizeof(unsigned char));
		m_idx.clear();
		unsigned int nsp = m_spBySensor[sens].size();
		for (unsigned int j = 1; j < nsp; ++j) {
			unsigned int spw = m_spBySensor[sens][j];
			unsigned int idx = (spw >> 8);
			m_buffer[idx] = spw & 0xFFU;
			m_idx.push_back(j);
		}
		for (unsigned int ik = 0; ik < m_idx.size(); ++ik) {
			const unsigned int spw = m_spBySensor[sens][m_idx[ik]];
			const unsigned int idx = (spw >> 8);
			const unsigned int row = idx & 0x3FU;
			const unsigned int col = (idx >> 6);
			unsigned int no_neighbour = 1;
			for (unsigned int ni = 0; ni < 8; ++ni) {
				const int nrow = row + dy[ni];
				if (nrow < 0 || nrow > 63) continue;
				const int ncol = col + dx[ni];
				if (ncol < 0 || ncol > 383) continue;
				const unsigned int nidx = (ncol << 6) | nrow;
				if (m_buffer[nidx]) {
					no_neighbour = 0;
					break;
				}
			}
			m_spBySensor[sens][m_idx[ik]] = spw | (no_neighbour << 31);
		}
	}

	int total = 0;
	for(unsigned int j=0; j<spByChip.size(); j++){
		spByChip[j][0] = spByChip[j].size()-1; // set correct size for first element
		unsigned int banksize = sizeof(unsigned int)*spByChip[j].size();
		total += banksize;
		fwrite(&m_evt, sizeof(unsigned int), 1, f);
		fwrite(&j, sizeof(unsigned int), 1, f);
		fwrite(&banksize, sizeof(unsigned int), 1, f);
		fwrite(&(spByChip[j][0]), banksize, 1, f);
		if(m_isDebug) debug() << "evt " << m_evt << " chip " << j << " sp " << spByChip[j].size() << endmsg;

                
                h_nsps->Fill(j, spByChip[j].size());

	}

	if(m_isDebug) debug() << "total " << total << endmsg;

	total = 0;
	for (unsigned int sensor = 0; sensor < m_spBySensor.size(); sensor++) {

		// encode header.
		m_spBySensor[sensor][0] =
			m_spBySensor[sensor].size() - 1;  // set correct number of super pixels

		unsigned int banksize = sizeof(unsigned int) * m_spBySensor[sensor].size();
		total += banksize;
		if (m_isDebug)
			debug() << "evt " << m_evt << " sensor " << sensor << " sp "
				<< m_spBySensor[sensor].size() << endmsg;

		LHCb::RawBank* spBank =
			rawEvent->createBank(sensor, LHCb::RawBank::VP, m_bankVersion, banksize,
					&(m_spBySensor[sensor][0]));
		// Add new bank and pass memory ownership to raw event
		rawEvent->adoptBank(spBank, true);
	}

	if (m_isDebug) debug() << "total " << total << endmsg;

	return StatusCode::SUCCESS;
}

StatusCode VPSuperPixelBankEncoder::finalize() {
	StatusCode sc = GaudiAlgorithm::finalize();
	fclose(f);
        histfile->Write();
        histfile->Close();

	info() << "Total Hits " << numHits << " SPs " << numSPs << endmsg;
	return StatusCode::SUCCESS;
}

int bunch_scheme(int bxid)
{
	// see richards training slides - called basic concepts
	// bunch train pattern - 234 334 334 334
	
	int t1 = 12; // ps bunch gap
	int t2 = 8;  // sps injection kicker
	int t3 = 38; // lhc injection kicker
	int t4 = 39; //  same
	int t5 = 119; // lhc beam dump kicker

	//int *scheme = { 2,3,4, 3,3,4, 3,3,4, 3,3,4 };
	
	//int *scheme = {72,8,72,8+30, 72,8,72,8,72,8+30, 72,8,72,8,72,8,72,8+31,
 		//72,8,72,8,72,8+30, 72,8,72,8,72,8+30, 72,8,72,8,72,8,72,8+31,
 		//72,8,72,8,72,8+30, 72,8,72,8,72,8+30, 72,8,72,8,72,8,72,8+31,	
 		//72,8,72,8,72,8+30, 72,8,72,8,72,8+30, 72,8,72,8,72,8,72,8+31+80};	
	int len = 2*(2+3+4 + 3+3+4 + 3+3+4 + 3+3+4);
	int filled[len];// = {72,8,72,8+30, 72,8,72,8,72,8+30, 72,8,72,8,72,8,72,8+31,

// 3564 = 2*(72b+8e) + 30e + 3*(72b+8e)+30e+ 4*(72b+8e) + 31e + 3*(2*(3*(72b+8e) +30e) +4*(72b+8e)+31e) + 80e
	int jump = 0;
	int full = 0;

	for(int i = 0; i<len; i++){
		filled[i] = (i+1)%2;
	}
	for(int i = 0; i<3564; i++){
		if (bxid < 72){
			full = 1;
		} else if (bxid < 8+72){
			full = 1;
		}
					 
	}

	
	return 1;
}
