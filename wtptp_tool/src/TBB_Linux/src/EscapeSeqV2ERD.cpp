/******************************************************************************
 *
 *  (C)Copyright 2012 - 2013 Marvell. All Rights Reserved.
 *  
 *  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF MARVELL.
 *  The copyright notice above does not evidence any actual or intended 
 *  publication of such source code.
 *  This Module contains Proprietary Information of Marvell and should be
 *  treated as Confidential.
 *  The information in this file is provided for the exclusive use of the 
 *  licensees of Marvell.
 *  Such users have the right to use, modify, and incorporate this code into 
 *  products for purposes authorized by the license agreement provided they 
 *  include this notice and the associated copyright notice with any such
 *  product. 
 *  The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/
 /********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.

	* Neither the name of Marvell nor the names of its contributors may be
	  used to endorse or promote products derived from this software without
	  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include <algorithm>

#include "EscapeSeqV2ERD.h"
#include "TimDescriptorParser.h"

const string CMFPRPair::Begin("MFPR Pair");
const string CMFPRPair::End("End MFPR Pair");

CMFPRPair::CMFPRPair() :
		CErdBase(MFPR_PAIR_ERD, MFPR_PAIR_MAX) {
	*m_FieldNames[MFPR_ADDRESS_FIELD] = "ADDRESS";
	*m_FieldNames[MFPR_VALUE_FIELD] = "VALUE";
}

CMFPRPair::~CMFPRPair() {

}

// copy constructor
CMFPRPair::CMFPRPair(const CMFPRPair& rhs) :
		CErdBase(rhs) {
	// copy constructor
}

// assignment operator
CMFPRPair& CMFPRPair::operator=(const CMFPRPair& rhs) {
	// assignment operator
	if (&rhs != this) {
		CErdBase::operator=(rhs);
	}
	return *this;
}

bool CMFPRPair::ToBinary(ofstream& ofs) {
	// validate size
	if (m_FieldValues.size() != m_iMaxFieldNum)
		return false;

	ofs << Translate(*m_FieldValues[MFPR_ADDRESS_FIELD]);
	ofs << Translate(*m_FieldValues[MFPR_VALUE_FIELD]);

	return ofs.good();
}

int CMFPRPair::AddPkgStrings(CReservedPackageData* pRPD) {
	pRPD->AddData(new string(*m_FieldValues[MFPR_ADDRESS_FIELD]),
			new string("ADDRESS"));
	pRPD->AddData(new string(*m_FieldValues[MFPR_VALUE_FIELD]),
			new string("VALUE"));

	return PackageSize();
}

bool CMFPRPair::Parse(CTimDescriptor& TimDescriptor, CTimDescriptorLine*& pLine,
		bool bIsBlf, string& sNum) {
	m_bChanged = false;

	while ((pLine = TimDescriptor.GetNextLineField(pLine))) {
		if (pLine->m_FieldName
				== (bIsBlf ?
						sNum + SpaceToUnderscore("End " + PackageName()) :
						("End " + PackageName()))) {
			return true;
		}

		bool bFound = false;
		for (unsigned int idx = 0; idx < m_FieldNames.size(); idx++) {
			if (pLine->m_FieldName
					== (bIsBlf ?
							sNum + SpaceToUnderscore(*m_FieldNames[idx]) :
							*m_FieldNames[idx])) {
				*m_FieldValues[idx] = pLine->m_FieldValue;
				bFound = true;
				break;
			}
		}

		if (!bFound) {
			stringstream ss;
			ss << endl << "Error: Parsing of '"
					<< (bIsBlf ?
							sNum + SpaceToUnderscore(PackageName()) :
							PackageName());
			TimDescriptor.ParsingError(ss.str(), true, pLine);
			break;
		}
	}

	// field not found  
	return true;
}

const string CEscapeSeqV2::Begin("Escape Seq V2");
const string CEscapeSeqV2::End("End Escape Seq V2");

CEscapeSeqV2::CEscapeSeqV2() :
		CErdBase(ESC_SEQ_V2_ERD, ESC_SEQ_V2_MAX) {
	enum EscapeSeqV2 {
		GPIO_NUM_FIELD,             // GPIO to check
		GPIO_TRIGGER_VALUE_FIELD, // value of GPIO to trigger the escape sequence
		NUM_MFPR_PAIRS,
		ESC_SEQ_V2_MAX
	};

	*m_FieldNames[GPIO_NUM_FIELD] = "GPIO_NUM";
	*m_FieldNames[GPIO_TRIGGER_VALUE_FIELD] = "GPIO_TRIGGER_VALUE";
	*m_FieldNames[NUM_MFPR_PAIRS] = "NUM_MFPR_PAIRS";
}

CEscapeSeqV2::~CEscapeSeqV2() {
	Reset();
}

// copy constructor
CEscapeSeqV2::CEscapeSeqV2(const CEscapeSeqV2& rhs) :
		CErdBase(rhs) {

	CEscapeSeqV2& nc_rhs = const_cast<CEscapeSeqV2&>(rhs);

	// copy constructor
	t_MFPRPairListIter iter = nc_rhs.m_MFPRPairs.begin();
	while (iter != nc_rhs.m_MFPRPairs.end()) {
		CMFPRPair* pMFPRPair = new CMFPRPair(*(*iter));
		m_MFPRPairs.push_back(pMFPRPair);

		iter++;
	}
}

// assignment operator
CEscapeSeqV2& CEscapeSeqV2::operator=(const CEscapeSeqV2& rhs) {
	// assignment operator
	if (&rhs != this) {
		CErdBase::operator=(rhs);

		Reset();

		CEscapeSeqV2& nc_rhs = const_cast<CEscapeSeqV2&>(rhs);

		// copy constructor
		t_MFPRPairListIter iter = nc_rhs.m_MFPRPairs.begin();
		while (iter != nc_rhs.m_MFPRPairs.end()) {
			CMFPRPair* pMFPRPair = new CMFPRPair(*(*iter));
			m_MFPRPairs.push_back(pMFPRPair);
			iter++;
		}
	}

	return *this;
}

void CEscapeSeqV2::Reset() {
	if (m_MFPRPairs.size() > 0) {
		for_each(m_MFPRPairs.begin(), m_MFPRPairs.end(),
				[](CMFPRPair*& pPair) {delete pPair;});
		m_MFPRPairs.clear();
	}
}

bool CEscapeSeqV2::Parse(CTimDescriptor& TimDescriptor,
		CTimDescriptorLine*& pLine, bool bIsBlf) {
	m_bChanged = false;

	if (pLine->m_FieldName != (bIsBlf ? SpaceToUnderscore(Begin) : Begin)) {
		stringstream ss;
		ss << endl << "Error: Parsing of '"
				<< (bIsBlf ? SpaceToUnderscore(Begin) : Begin);
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	while ((pLine = TimDescriptor.GetNextLineField(pLine))) {
		if (pLine->m_FieldName == (bIsBlf ? SpaceToUnderscore(End) : End)) {
			break;
		}

		bool bFound = false;
		for (unsigned int idx = 0; idx < m_FieldNames.size(); idx++) {
			if (pLine->m_FieldName == *m_FieldNames[idx]) {
				*m_FieldValues[idx] = pLine->m_FieldValue;
				bFound = true;
				break;
			}
		}

		if (bFound)
			continue;

		stringstream ssMFPRPairNum;
		int nMFPRPair = m_MFPRPairs.size() + 1;
		ssMFPRPairNum.str("");
		ssMFPRPairNum << nMFPRPair << "_";

		CMFPRPair* pMFPRPair = 0;
		if (pLine->m_FieldName
				== (bIsBlf ?
						ssMFPRPairNum.str()
								+ SpaceToUnderscore(CMFPRPair::Begin) :
						CMFPRPair::Begin)) {
			pMFPRPair = new CMFPRPair;
			if (pMFPRPair) {
				string sMFPRPair = ssMFPRPairNum.str(); // this syntax if for linux compiler
				if (!pMFPRPair->Parse(TimDescriptor, pLine, bIsBlf,
						sMFPRPair)) {
					stringstream ss;
					ss << endl << "Error: Parsing of '"
							<< (bIsBlf ?
									ssMFPRPairNum.str()
											+ SpaceToUnderscore(
													CMFPRPair::Begin) :
									CMFPRPair::Begin);
					TimDescriptor.ParsingError(ss.str(), true, pLine);
					delete pMFPRPair;
					return false;
				}
				if (pMFPRPair
						&& pLine->m_FieldName
								== (bIsBlf ?
										ssMFPRPairNum.str()
												+ SpaceToUnderscore(
														CMFPRPair::End) :
										CMFPRPair::End)) {
					m_MFPRPairs.push_back(pMFPRPair);
					pMFPRPair = 0;
					continue;
				}
			}
		}

		stringstream ss;
		ss << endl << "Error: Parsing of '"
				<< (bIsBlf ? ssMFPRPairNum.str() + SpaceToUnderscore(End) : End);
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		delete pMFPRPair;
		return false;
	}

	if (Translate(*m_FieldValues[NUM_MFPR_PAIRS]) != m_MFPRPairs.size()) {
		stringstream ss;
		ss << endl << "Error: Parsing of '"
				<< (bIsBlf ? SpaceToUnderscore(Begin) : Begin)
				<< ", NUM_MFPR_PAIRS not equal to actual number of MFPR Pair defined in the Escape Seq V2";
		TimDescriptor.ParsingError(ss.str(), true, pLine);
		return false;
	}

	return true;
}

bool CEscapeSeqV2::ToBinary(ofstream& ofs) {
	// validate size
	if (m_FieldValues.size() != m_iMaxFieldNum)
		return false;

	ofs << ESCSEQID_V2;
	ofs << PackageSize();
	ofs << Translate(*m_FieldValues[GPIO_NUM_FIELD]);
	ofs << Translate(*m_FieldValues[GPIO_TRIGGER_VALUE_FIELD]);
	ofs << Translate(*m_FieldValues[NUM_MFPR_PAIRS]);

	bool bRet = true;

	for_each(m_MFPRPairs.begin(), m_MFPRPairs.end(),
			[ &ofs ](CMFPRPair*& pPair) {pPair->ToBinary(ofs);});
	return (ofs.good() && bRet);
}

int CEscapeSeqV2::AddPkgStrings(CReservedPackageData* pRPD) {
	pRPD->PackageIdTag(HexFormattedAscii(ESCSEQID_V2));
	pRPD->AddData(new string(*m_FieldValues[GPIO_NUM_FIELD]),
			new string("GPIO_NUM"));
	pRPD->AddData(new string(*m_FieldValues[GPIO_TRIGGER_VALUE_FIELD]),
			new string("GPIO_TRIGGER_VALUE"));
	pRPD->AddData(new string(*m_FieldValues[NUM_MFPR_PAIRS]),
			new string("NUM_MFPR_PAIRS"));

	for_each(m_MFPRPairs.begin(), m_MFPRPairs.end(),
			[&pRPD](CMFPRPair*& pPair) {pPair->AddPkgStrings(pRPD);});
	return PackageSize();
}

unsigned int CEscapeSeqV2::PackageSize() {
	return (unsigned int) (8 + // package tag id + size
			(m_FieldValues.size() * 4) // all fields
			+ m_MFPRPairs.size() * 8);
}
