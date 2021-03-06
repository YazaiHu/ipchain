﻿#ifndef	   _IPCCHAIN_CONSENSUSACCOUNTPOOL_H_201708111059_
#define    _IPCCHAIN_CONSENSUSACCOUNTPOOL_H_201708111059_

#include <list>
#include <mutex>
#include <map>
#include <set>
#include <utility>
#include "ConsensusAccount.h"

/** Cache block length */
static const int CACHED_BLOCK_COUNT = 10;
static const int MAX_TIMEOUT_COUNT = 3;	//Continuous no block number, overtime penalty
static const int MAX_BLOCK_TIME_DIFF = 5; //The actual time of block and calculation time is the maximum gap, the unit is seconds
static const int PACKAGE_CREDIT_REWARD = 1; //A credit bonus for each account
static const int TIMEOUT_CREDIT_PUNISH = 100;//Each omission of the credit worthiness penalty
static const int MIN_PERIOD_COUNT = 20;		//Dynamic adjustment of deposit, meeting (MAX [n thousand, current maximum block]) when the number of accounts is less than that, the deposit is the minimum deposit
static const int MAX_PERIOD_COUNT = 100;	//The amount of the deposit will reach the IPC circulation when the number of accounts of the meeting is reached, and the curve is (x/maximum number) squared or the fourth power

//Cache block length retained in memory
#define SNAPSHOTLENGTH  20000//50  //20000
#define SNAPSHOTINSERT  1000//25  //000
#define SPLITEDSNAPSHOTFILENOMBER 0


class SnapshotClass {
public:
	uint16_t pkHashIndex;			//Record the block public key index of the corresponding block
	uint32_t blockHeight;			//Record the block height of the corresponding block
	uint64_t timestamp;				//Record the block time of the corresponding block (quasi, second level)
	uint64_t meetingstarttime;		
	uint64_t meetingstoptime;		
	uint32_t blockTime;				//Record the actual packaging time of the corresponding block

	std::set<uint16_t> curCandidateIndexList;			//Current round candidate list (index value, accountlist)
	std::set<uint16_t> curSeriousPunishIndexAdded;		//Current round of additional severe punishment (index value, not accumulated)
	std::map<uint16_t, int> curTimeoutIndexRecord;		//Current round update timeout record (index, consecutive no block times)
	std::set<uint16_t> curRefundIndexList;				//Current round new demand for refund (index value, not accumulated)
	std::set<uint16_t> curTimeoutPunishList;			//Current round new demand for overtime penalty (index value, not cumulative)
	std::set<uint16_t> curBanList;						//Current wheel blacklist (index value, not cumulative)

	//From the cur list before the N round and the last one, 
	//and the transactions included in the received block, 
	//the refund index needed to be packaged/checked for the next block is calculated
	std::set<uint16_t> cachedIndexsToRefund;

	//from the previous round of status, 
	//and the transactions included in the received block, 
	//the refund index needed to be packaged/checked for the next block is calculated
	std::set<uint16_t> cachedTimeoutPunishToRun;

	// cache when the front runner list (based on the list of candidates to be calculated by push)
	std::vector<std::pair<uint16_t, int64_t>> cachedMeetingAccounts;	
	//Current round blacklist(index value, cumulative, never empty)
	std::set<uint16_t> cachedBanList;

	SnapshotClass();
	SnapshotClass(const SnapshotClass& in);
	~SnapshotClass() {};

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action) {
		READWRITE(pkHashIndex);
		READWRITE(blockHeight);
		READWRITE(timestamp);
		READWRITE(meetingstarttime);
		READWRITE(meetingstoptime);
		READWRITE(blockTime);

		READWRITE(curCandidateIndexList);
		READWRITE(curSeriousPunishIndexAdded);
		READWRITE(curTimeoutIndexRecord);
		READWRITE(curRefundIndexList);
		READWRITE(curTimeoutPunishList);
		READWRITE(curBanList);

		READWRITE(cachedIndexsToRefund);
		READWRITE(cachedTimeoutPunishToRun);
		READWRITE(cachedMeetingAccounts);
		READWRITE(cachedBanList);
	}

	bool operator==( const SnapshotClass& other)
	{
		return (curCandidateIndexList == other.curCandidateIndexList &&
			curSeriousPunishIndexAdded == other.curSeriousPunishIndexAdded &&
			curTimeoutIndexRecord == other.curTimeoutIndexRecord &&
			curRefundIndexList == other.curRefundIndexList &&
			curTimeoutPunishList == other.curTimeoutPunishList &&
			curBanList == other.curBanList &&
			cachedIndexsToRefund == other.cachedIndexsToRefund &&
			cachedTimeoutPunishToRun == other.cachedTimeoutPunishToRun &&
			cachedMeetingAccounts == other.cachedMeetingAccounts &&
			cachedBanList == other.cachedBanList);
		
	}

	void compressclear() {
		curCandidateIndexList.clear();
		curSeriousPunishIndexAdded.clear();
		curTimeoutIndexRecord.clear();
		curRefundIndexList.clear();
		curTimeoutPunishList.clear();
		curBanList.clear();
		cachedIndexsToRefund.clear();
		cachedTimeoutPunishToRun.clear();
		cachedMeetingAccounts.clear();
		cachedBanList.clear();
	};
};

enum DPOC_errtype {
	EXIT_PUBKEY_NOT_EXIST_IN_LIST = 0,
	EXIT_UNKNOWN_PUBKEY,
	JOIN_PUBKEY_IS_DEPOSING,
	JOIN_PUBKEY_IS_TIMEOUT_PUNISHED,
	GET_CACHED_SNAPSHOT_FAILED,
	EXIT_PUBKEY_IS_DEPOSING,
	EXIT_PUBKEY_IS_TIMEOUT_PUNISHED,
	JOIN_PUBKEY_ALREADY_EXIST_IN_LIST,
	JOIN_TRANS_DEPOSI_TOO_SMALL,
	GET_LAST_SNAPSHOT_FAILED,
	EVIDENCE_ISEMPTY,
	JOIN_PUBKEY_IS_BANNED,
	UNKNOWN_PACKAGE_PUBKEY,
	BLOCK_TOO_NEW_FOR_SNAPSHOT,
	PUNISH_BLOCK,
	PUBKEY_IS_BANNED,
};

class CConsensusAccountPool {
public:
	void printshopsize();
	bool verifyDPOCTx(const CTransaction& tx, DPOC_errtype &errorType);
	bool checkNewBlock(const std::shared_ptr<const CBlock> pblock, uint160 &pubkeyHash, uint32_t blockHeight, DPOC_errtype & errorType);
	bool verifyDPOCBlock(const std::shared_ptr<const CBlock> pblock, uint32_t blockHeight, DPOC_errtype &rejectreason);
	bool pushDPOCBlock(const std::shared_ptr<const CBlock> pblock, uint32_t blockHeight);
	bool popDPOCBlock(uint32_t blockHeight);
	//According to the cache of the current list, the refund, penalty transaction is added to the block, and the block is signed
	bool AddDPOCCoinbaseToBlock(CBlock* pblockNew, CBlockIndex* pindexPrev, uint32_t blockHeight, CMutableTransaction &coinbaseTx);
	//Signature function
	bool addSignToCoinBase(CBlock* pblock, CBlockIndex* pindexPrev, uint32_t blockHeight, CMutableTransaction &coinbaseTx, uint160* pPubKeyHash);
	//Analyze the CoinBase acquisition of the received block, Publickey, and signature fields
	bool getPublicKeyFromBlock(const CBlock *pblock, CPubKey& outPubkey, std::vector<unsigned char>& outRecvSign);
	bool getPublicKeyFromSignstring(const std::string signstr, CPubKey& outPubkey, std::vector<unsigned char>& outRecvSign);
	bool ContainPK(uint160 pk, uint16_t& index);
	bool contain(std::vector<std::pair<uint16_t, int64_t>> list, uint16_t indexIn);
	bool getCreditFromSnapshotByIndex(SnapshotClass snapshot, const uint16_t indexIn, int64_t &credit);
	//Determine whether the deposit that applies to the consensus is thawed
	bool IsAviableUTXO(const uint256 hash);
	CAmount GetDepositBypkhash(uint160 pkhash);
	uint256 GetTXhashBypkhash(uint160 pkhash);
	//Snapshot related read and write operations
	bool GetSnapshotByTime(SnapshotClass &snapshot, uint64_t readtime);
	bool GetSnapshotByHeight(SnapshotClass &snapshot, uint32_t height);
	bool GetSnapshotsByHeight(std::vector<SnapshotClass> &snapshots, uint32_t lowest, uint32_t highest = 0);
	//Gets the snapshot at the end of the current snapshot list
	bool GetLastSnapshot(SnapshotClass &snapshot);
	//Add the snapshot to the end of the snapshot list
	bool PushSnapshot(SnapshotClass snapshot);
	bool getPKIndexBySortedIndex(uint16_t &pkIndex, uint160 &pkhash, std::list<std::shared_ptr<CConsensusAccount >> consensusList, int sortedIndex);
	bool GetTimeoutIndexs(const std::shared_ptr<const CBlock> pblock, SnapshotClass lastsnapshot, std::map<uint16_t, int>& timeoutindexs);
	CAmount GetCurDepositAdjust(uint160 pkhash,uint32_t blockheight);
	bool SetConsensusStatus(const std::string &strStatus, const std::string &strHash);
	uint64_t getCSnapshotIndexSize();
	uint64_t getSnapshotSize(SnapshotClass &snapshot);
	void writeCandidatelistToFileByHeight(uint32_t nHeight);
	bool writeCandidatelistToFile(const CConsensusAccount &account);
	bool readCandidatelistFromFile();
	uint64_t getConsensAccountSize();
	void writeCandidatelistToFile();
	void flushSnapshotToDisk();

	bool rollbackCandidatelist(uint32_t nHeight);
	bool verifyBlockSign(const CBlock *pblock);
	bool analysisConsensusSnapshots();
	bool InitFinished()
	{
		return analysisfinished;
	};
	//The session invokes the operation to return a list of candidates
	bool listSnapshots(std::list<std::shared_ptr<CConsensusAccount>> &list);
	bool listSnapshotsToTime(std::list<std::shared_ptr<CConsensusAccount>> &list, int readtime);
	void getTrustList(std::vector<std::string> &trustList);
	//According to BlockIndex, publickey is given publickey
	bool getSignPkByBlockIndex(const CBlockIndex *pindexTest, CKeyID &pubicKey160hash);
	bool getConsensusListByBlock(const CBlock& block, std::set<uint16_t> &setAccounts, std::list<std::shared_ptr<CConsensusAccount >> &consensusList);
	bool checkPackagerInCurrentList(const CBlock& block);
	bool setTotalAmount(const CBlock &block,bool bAdd);
	bool verifyPkInCandidateListByTime(int64_t curStarttime, CKeyID &pubicKey160hash);
	bool verifyPkInCandidateList(const std::shared_ptr<const CBlock> pblock, CKeyID &pubicKey160hash);
	bool verifyPkInCandidateListByIndex(CBlockIndex *pIndex, CKeyID &pubicKey160hash);
	//Determines whether the public key is a trusted node
	bool verifyPkIsTrustNode(std::string strPublicKey);
	bool verifyPkIsTrustNode(CKeyID  &pubicKey160hash);
	bool getCreditbyPkhash(uint160 pkhash, int64_t &n64Credit);
	
	static  CConsensusAccountPool&  Instance();
	~CConsensusAccountPool();

private:
	void printsnapshots(std::vector<SnapshotClass> list);
	void printsets(std::set<uint16_t> list);
	bool setSnapshotFilePath(int nNum);
	bool truncateSnapshotFile(uint32_t u32OldHeight, uint32_t u32NowHeight);

	bool splitSnapshotFile();
	void getSplitedSnapshotAndIndexFileSize(const std::string strSnapshotIndexPath, const uint64_t nIndex,
		uint64_t & nOldSnapshotFileOffset, uint64_t & nOldSnapshotIndexFileOffset,
		uint64_t & nSnapshotNewFileSize, uint64_t & nSnapshotIndexNewFileSize);
	bool copyPartofFile(const uint64_t nSize, FILE *fileOld, FILE *fileNew);
	bool createSplitedFile(const std::string strSnapshotPath, const uint64_t nSnapshotNewFileSize, FILE *fileSnapshotOld);
	bool createSplitedSnapshotIndexFile(std::string strSnapshotIndexPath, const uint64_t nSnapshotIndexNewFileSize, FILE *fileSnapshotIndexOld);
	bool createSplitedSnapshotAndIndexFile(const int nFileNum, const uint64_t nSnapshotNewFileSize, const uint64_t nSnapshotIndexNewFileSize,
		                               FILE *fileSnapshotOld, FILE *fileSnapshotIndexOld);
private:
	CConsensusAccountPool();
	static void CreateInstance();
	static CConsensusAccountPool* _instance;
	static std::once_flag init_flag;

	std::mutex poolMutex;
	//Store the list of Shared Shared accounts
	std::list<std::unique_ptr<CConsensusAccount>> m_listConsensusAccount;
	//Create a blacklist and store serious offending nodes
	std::list<std::unique_ptr<CConsensusAccount>> m_ListCriticalViolation;
	//Create a list of common punishments
	std::list<std::unique_ptr<CConsensusAccount>> m_ListNormalViolation;
	typedef boost::shared_lock<boost::shared_mutex> readLock;
	typedef boost::unique_lock<boost::shared_mutex> writeLock;
	boost::shared_mutex rwmutex;
	std::map<uint32_t, SnapshotClass> snapshotlist; //key is block Height
	std::map<uint32_t, uint64_t> m_mapSnapshotIndex;
	std::string m_strSnapshotPath;
	std::string m_strSnapshotDir;
	std::string m_strSnapshotIndexPath;
	//std::string m_strSnapshotRevPath;
	std::string m_strCandidatelistPath;

	uint64_t m_u64SnapshotIndexSize;
	uint64_t m_u64ConsensAccountSize;
	uint32_t m_u32WriteHeight;
	//Consensus list, the consensus of the consensus person's HASH
	std::vector<CConsensusAccount> candidatelist; 													
	std::set<uint16_t> tmpcachedIndexsToRefund;  
	std::set<uint16_t> tmpcachedTimeoutToPunish;
	bool verifysuccessed;
	//The analysis of local block completion is true
	bool analysisfinished;
	//The public key list that starts with the public key and will not be penalized in the future
	std::vector<std::string> trustPKHashList; 
	bool bReboot;
	
};
#endif
