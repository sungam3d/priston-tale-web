#define	FACE_MODEL_MAX		7	// 전업전[0], 1차전업[1], 2차전업[2],3차전업[3], 얼큰버젼 2가지[4],[5] // 박재원 - 2010 월드컵 이벤트(축구공 포션 추가[6]) - 총 7가지
#define	HEAREPOTION_MAX		13	//헤어틴트포션 최대치  < 060809 헤어틴트포션  성근 수정  >

////////////////////////// 모델 이미지 정리 /////////////////////////////
char *szModel_FighterFaceName[FACE_MODEL_MAX][HEAREPOTION_MAX] = {				//파이터
	{
		// 전업전[0]
		"char\\tmABCD\\tmh-b01.inf",				
		"char\\tmABCD\\tmh-b02.inf",
		"char\\tmABCD\\tmh-b03.inf",
		"char\\tmABCD\\tmh-b04.inf",
		"char\\tmABCD\\tmh-b05.inf",
		"char\\tmABCD\\tmh-b06.inf",
		"char\\tmABCD\\tmh-b07.inf",
		"char\\tmABCD\\tmh-b08.inf",	

		"char\\tmABCD\\tmh-b09.inf",
		"char\\tmABCD\\tmh-b10.inf",
		"char\\tmABCD\\tmh-b11.inf",
		"char\\tmABCD\\tmh-b12.inf",
		"char\\tmABCD\\tmh-b13.inf"	

	},
	{
		// 1차 전업
		"char\\tmABCD\\tmh-b01a.inf",
		"char\\tmABCD\\tmh-b02a.inf",
		"char\\tmABCD\\tmh-b03a.inf",
		"char\\tmABCD\\tmh-b04a.inf",
		"char\\tmABCD\\tmh-b05a.inf",
		"char\\tmABCD\\tmh-b06a.inf",
		"char\\tmABCD\\tmh-b07a.inf",
		"char\\tmABCD\\tmh-b08a.inf",

		"char\\tmABCD\\tmh-b09a.inf",
		"char\\tmABCD\\tmh-b10a.inf",
		"char\\tmABCD\\tmh-b11a.inf",
		"char\\tmABCD\\tmh-b12a.inf",
		"char\\tmABCD\\tmh-b13a.inf"
	
	},
	{
		// 2차 전업
		"char\\tmABCD\\tmh_b01b.inf",
		"char\\tmABCD\\tmh_b02b.inf",
		"char\\tmABCD\\tmh_b03b.inf",
		"char\\tmABCD\\tmh_b04b.inf",
		"char\\tmABCD\\tmh_b05b.inf",
		"char\\tmABCD\\tmh_b06b.inf",
		"char\\tmABCD\\tmh_b07b.inf",
		"char\\tmABCD\\tmh_b08b.inf",
		
		"char\\tmABCD\\tmh_b09b.inf",
		"char\\tmABCD\\tmh_b10b.inf",
		"char\\tmABCD\\tmh_b11b.inf",
		"char\\tmABCD\\tmh_b12b.inf",
		"char\\tmABCD\\tmh_b13b.inf"
	
	},
	{
		// 3차 전업
		"char\\tmABCD\\tmh-b01c.inf",
		"char\\tmABCD\\tmh-b02c.inf",
		"char\\tmABCD\\tmh-b03c.inf",
		"char\\tmABCD\\tmh-b04c.inf",
		"char\\tmABCD\\tmh-b05c.inf",
		"char\\tmABCD\\tmh-b06c.inf",
		"char\\tmABCD\\tmh-b07c.inf",
		"char\\tmABCD\\tmh-b08c.inf",

		"char\\tmABCD\\tmh-b09c.inf",
		"char\\tmABCD\\tmh-b10c.inf",
		"char\\tmABCD\\tmh-b11c.inf",
		"char\\tmABCD\\tmh-b12c.inf",
		"char\\tmABCD\\tmh-b13c.inf"
	
	},

	//////////// 얼큰 버전 - 1 //////////
	{
		"char\\tmABCD\\tmh-b01b.inf",
		"char\\tmABCD\\tmh-b02b.inf",
		"char\\tmABCD\\tmh-b03b.inf",
		
		"char\\tmABCD\\tmh-b01b.inf",
		"char\\tmABCD\\tmh-b02b.inf",
		"char\\tmABCD\\tmh-b03b.inf",
		"char\\tmABCD\\tmh-b01b.inf",
		"char\\tmABCD\\tmh-b02b.inf",
		
		"char\\tmABCD\\tmh-b03b.inf",
		"char\\tmABCD\\tmh-b01b.inf",
		"char\\tmABCD\\tmh-b02b.inf",
		"char\\tmABCD\\tmh-b03b.inf",
		"char\\tmABCD\\tmh-b01b.inf"	

	},

	//////////// 얼큰 버전 - 2 //////////
	{
		"char\\tmABCD\\tmh-b01t.inf",
		"char\\tmABCD\\tmh-b02t.inf",
		"char\\tmABCD\\tmh-b03t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\tmh-b01t.inf",
		"char\\tmABCD\\tmh-b02t.inf",
		"char\\tmABCD\\tmh-b03t.inf",
		"char\\tmABCD\\tmh-b01t.inf",
		"char\\tmABCD\\tmh-b02t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\tmh-b01t.inf",
		"char\\tmABCD\\tmh-b02t.inf",
		"char\\tmABCD\\tmh-b03t.inf",
		"char\\tmABCD\\tmh-b01t.inf",
		"char\\tmABCD\\tmh-b02t.inf"	
	},

	// 박재원 - 2010 월드컵 이벤트 - 축구공 포션 추가(축구공 모델 이미지 추가 - 머리)
	{
		"char\\tmABCD\\TmhB-18t.inf",
		"char\\tmABCD\\TmhB-18t.inf",
		"char\\tmABCD\\TmhB-18t.inf",

		"char\\tmABCD\\TmhB-18t.inf",
		"char\\tmABCD\\TmhB-18t.inf",
		"char\\tmABCD\\TmhB-18t.inf",
		"char\\tmABCD\\TmhB-18t.inf",
		"char\\tmABCD\\TmhB-18t.inf",

		"char\\tmABCD\\TmhB-18t.inf",
		"char\\tmABCD\\TmhB-18t.inf",
		"char\\tmABCD\\TmhB-18t.inf",
		"char\\tmABCD\\TmhB-18t.inf",
		"char\\tmABCD\\TmhB-18t.inf"	
	}
};

char *szModel_MechFaceName[FACE_MODEL_MAX][HEAREPOTION_MAX] = {			//메카니션
	{
		"char\\tmABCD\\tmh-a01.inf",
		"char\\tmABCD\\tmh-a02.inf",
		"char\\tmABCD\\tmh-a03.inf",
		"char\\tmABCD\\tmh-a04.inf",
		"char\\tmABCD\\tmh-a05.inf",
		"char\\tmABCD\\tmh-a06.inf",
		"char\\tmABCD\\tmh-a07.inf",
		"char\\tmABCD\\tmh-a08.inf",
		"char\\tmABCD\\tmh-a09.inf",
		"char\\tmABCD\\tmh-a10.inf",
		"char\\tmABCD\\tmh-a11.inf",
		"char\\tmABCD\\tmh-a12.inf",
		"char\\tmABCD\\tmh-a13.inf"
	},
	{
		"char\\tmABCD\\tmh-a01a.inf",
		"char\\tmABCD\\tmh-a02a.inf",
		"char\\tmABCD\\tmh-a03a.inf",
		"char\\tmABCD\\tmh-a04a.inf",
		"char\\tmABCD\\tmh-a05a.inf",
		"char\\tmABCD\\tmh-a06a.inf",
		"char\\tmABCD\\tmh-a07a.inf",
		"char\\tmABCD\\tmh-a08a.inf",
		"char\\tmABCD\\tmh-a09a.inf",
		"char\\tmABCD\\tmh-a10a.inf",
		"char\\tmABCD\\tmh-a11a.inf",
		"char\\tmABCD\\tmh-a12a.inf",
		"char\\tmABCD\\tmh-a13a.inf"
	},
	{
		"char\\tmABCD\\tmh_a01b.inf",
		"char\\tmABCD\\tmh_a02b.inf",
		"char\\tmABCD\\tmh_a03b.inf",
		"char\\tmABCD\\tmh_a04b.inf",
		"char\\tmABCD\\tmh_a05b.inf",
		"char\\tmABCD\\tmh_a06b.inf",
		"char\\tmABCD\\tmh_a07b.inf",
		"char\\tmABCD\\tmh_a08b.inf",
		"char\\tmABCD\\tmh_a09b.inf",
		"char\\tmABCD\\tmh_a10b.inf",
		"char\\tmABCD\\tmh_a11b.inf",
		"char\\tmABCD\\tmh_a12b.inf",
		"char\\tmABCD\\tmh_a13b.inf"
	},
	{
		"char\\tmABCD\\tmh-a01c.inf",
		"char\\tmABCD\\tmh-a02c.inf",
		"char\\tmABCD\\tmh-a03c.inf",
		"char\\tmABCD\\tmh-a04c.inf",
		"char\\tmABCD\\tmh-a05c.inf",
		"char\\tmABCD\\tmh-a06c.inf",
		"char\\tmABCD\\tmh-a07c.inf",
		"char\\tmABCD\\tmh-a08c.inf",
		"char\\tmABCD\\tmh-a09c.inf",
		"char\\tmABCD\\tmh-a10c.inf",
		"char\\tmABCD\\tmh-a11c.inf",
		"char\\tmABCD\\tmh-a12c.inf",
		"char\\tmABCD\\tmh-a13c.inf"
	},
	//////////// 얼큰 버전 //////////
	{
		"char\\tmABCD\\tmh-a01b.inf",
		"char\\tmABCD\\tmh-a02b.inf",
		"char\\tmABCD\\tmh-a03b.inf",
		
		"char\\tmABCD\\tmh-a01b.inf",
		"char\\tmABCD\\tmh-a02b.inf",
		"char\\tmABCD\\tmh-a03b.inf",
		"char\\tmABCD\\tmh-a01b.inf",
		"char\\tmABCD\\tmh-a02b.inf",
		
		"char\\tmABCD\\tmh-a03b.inf",
		"char\\tmABCD\\tmh-a01b.inf",
		"char\\tmABCD\\tmh-a02b.inf",
		"char\\tmABCD\\tmh-a03b.inf",
		"char\\tmABCD\\tmh-a01b.inf"
		
	},
	{
		"char\\tmABCD\\tmh-a01t.inf",
		"char\\tmABCD\\tmh-a02t.inf",
		"char\\tmABCD\\tmh-a03t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\tmh-a01t.inf",
		"char\\tmABCD\\tmh-a02t.inf",
		"char\\tmABCD\\tmh-a03t.inf",
		"char\\tmABCD\\tmh-a01t.inf",
		"char\\tmABCD\\tmh-a02t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\tmh-a01t.inf",
		"char\\tmABCD\\tmh-a02t.inf",
		"char\\tmABCD\\tmh-a03t.inf",
		"char\\tmABCD\\tmh-a01t.inf",
		"char\\tmABCD\\tmh-a02t.inf"
	},
	
	// 박재원 - 2010 월드컵 이벤트 - 축구공 포션 추가(축구공 모델 이미지 추가 - 머리)
	{
		"char\\tmABCD\\TmhA-18t.inf",
		"char\\tmABCD\\TmhA-18t.inf",
		"char\\tmABCD\\TmhA-18t.inf",

		"char\\tmABCD\\TmhA-18t.inf",
		"char\\tmABCD\\TmhA-18t.inf",
		"char\\tmABCD\\TmhA-18t.inf",
		"char\\tmABCD\\TmhA-18t.inf",
		"char\\tmABCD\\TmhA-18t.inf",

		"char\\tmABCD\\TmhA-18t.inf",
		"char\\tmABCD\\TmhA-18t.inf",
		"char\\tmABCD\\TmhA-18t.inf",
		"char\\tmABCD\\TmhA-18t.inf",
		"char\\tmABCD\\TmhA-18t.inf"	
	}
};

char *szModel_ArcherFaceName[FACE_MODEL_MAX][HEAREPOTION_MAX] = {		//아쳐
	{	
		"char\\tmABCD\\tfh-D01.inf",
		"char\\tmABCD\\tfh-d02.inf",
		"char\\tmABCD\\tfh-d03.inf",
		"char\\tmABCD\\tfh-d04.inf",
		"char\\tmABCD\\tfh-d05.inf",
		"char\\tmABCD\\tfh-d06.inf",
		"char\\tmABCD\\tfh-d07.inf",
		"char\\tmABCD\\tfh-d08.inf",
		"char\\tmABCD\\tfh-d09.inf",
		"char\\tmABCD\\tfh-d10.inf",
		"char\\tmABCD\\tfh-d11.inf",
		"char\\tmABCD\\tfh-d12.inf",
		"char\\tmABCD\\tfh-d13.inf"
	},
	{
		"char\\tmABCD\\tfh-D01a.inf",
		"char\\tmABCD\\tfh-d02a.inf",
		"char\\tmABCD\\tfh-d03a.inf",
		"char\\tmABCD\\tfh-d04a.inf",
		"char\\tmABCD\\tfh-d05a.inf",
		"char\\tmABCD\\tfh-d06a.inf",
		"char\\tmABCD\\tfh-d07a.inf",
		"char\\tmABCD\\tfh-d08a.inf",
		"char\\tmABCD\\tfh-d09a.inf",
		"char\\tmABCD\\tfh-d10a.inf",
		"char\\tmABCD\\tfh-d11a.inf",
		"char\\tmABCD\\tfh-d12a.inf",
		"char\\tmABCD\\tfh-d13a.inf"
	},
	{
		"char\\tmABCD\\tfh_D01b.inf",
		"char\\tmABCD\\tfh_d02b.inf",
		"char\\tmABCD\\tfh_d03b.inf",
		"char\\tmABCD\\tfh_d04b.inf",
		"char\\tmABCD\\tfh_d05b.inf",
		"char\\tmABCD\\tfh_d06b.inf",
		"char\\tmABCD\\tfh_d07b.inf",
		"char\\tmABCD\\tfh_d08b.inf",
		"char\\tmABCD\\tfh_d09b.inf",
		"char\\tmABCD\\tfh_d10b.inf",
		"char\\tmABCD\\tfh_d11b.inf",
		"char\\tmABCD\\tfh_d12b.inf",
		"char\\tmABCD\\tfh_d13b.inf"
	},
	{
		"char\\tmABCD\\tfh-D01c.inf",
		"char\\tmABCD\\tfh-d02c.inf",
		"char\\tmABCD\\tfh-d03c.inf",
		"char\\tmABCD\\tfh-d04c.inf",
		"char\\tmABCD\\tfh-d05c.inf",
		"char\\tmABCD\\tfh-d06c.inf",
		"char\\tmABCD\\tfh-d07c.inf",
		"char\\tmABCD\\tfh-d08c.inf",
		"char\\tmABCD\\tfh-d09c.inf",
		"char\\tmABCD\\tfh-d10c.inf",
		"char\\tmABCD\\tfh-d11c.inf",
		"char\\tmABCD\\tfh-d12c.inf",
		"char\\tmABCD\\tfh-d13c.inf"
	},

	//////////// 얼큰 버전 //////////
	{
		"char\\tmABCD\\tfh-D01t.inf",
		"char\\tmABCD\\tfh-d02t.inf",
		"char\\tmABCD\\tfh-d03t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\tfh-D01t.inf",
		"char\\tmABCD\\tfh-d02t.inf",
		"char\\tmABCD\\tfh-d03t.inf",
		"char\\tmABCD\\tfh-D01t.inf",
		"char\\tmABCD\\tfh-d02t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\tfh-D01t.inf",
		"char\\tmABCD\\tfh-d02t.inf",
		"char\\tmABCD\\tfh-d03t.inf",
		"char\\tmABCD\\tfh-D01t.inf",
		"char\\tmABCD\\tfh-d02t.inf"

	},
	{
		"char\\tmABCD\\tfh-D01t.inf",
		"char\\tmABCD\\tfh-d02t.inf",
		"char\\tmABCD\\tfh-d03t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\tfh-D01t.inf",
		"char\\tmABCD\\tfh-d02t.inf",
		"char\\tmABCD\\tfh-d03t.inf",
		"char\\tmABCD\\tfh-D01t.inf",
		"char\\tmABCD\\tfh-d02t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\tfh-D01t.inf",
		"char\\tmABCD\\tfh-d02t.inf",
		"char\\tmABCD\\tfh-d03t.inf",
		"char\\tmABCD\\tfh-D01t.inf",
		"char\\tmABCD\\tfh-d02t.inf"
	},

	// 박재원 - 2010 월드컵 이벤트 - 축구공 포션 추가(축구공 모델 이미지 추가 - 머리)
	{
		"char\\tmABCD\\TfhD-18t.inf",
		"char\\tmABCD\\TfhD-18t.inf",
		"char\\tmABCD\\TfhD-18t.inf",

		"char\\tmABCD\\TfhD-18t.inf",
		"char\\tmABCD\\TfhD-18t.inf",
		"char\\tmABCD\\TfhD-18t.inf",
		"char\\tmABCD\\TfhD-18t.inf",
		"char\\tmABCD\\TfhD-18t.inf",

		"char\\tmABCD\\TfhD-18t.inf",
		"char\\tmABCD\\TfhD-18t.inf",
		"char\\tmABCD\\TfhD-18t.inf",
		"char\\tmABCD\\TfhD-18t.inf",
		"char\\tmABCD\\TfhD-18t.inf"	
	}
};

char *szModel_PikeFaceName[FACE_MODEL_MAX][HEAREPOTION_MAX] = {		//파이크맨
	{
		"char\\tmABCD\\tmh-c01.inf",		
		"char\\tmABCD\\tmh-c02.inf",		
		"char\\tmABCD\\tmh-c03.inf",		
		"char\\tmABCD\\tmh-c04.inf",		
		"char\\tmABCD\\tmh-c05.inf",		
		"char\\tmABCD\\tmh-c06.inf",		
		"char\\tmABCD\\tmh-c07.inf",		
		"char\\tmABCD\\tmh-c08.inf",
		"char\\tmABCD\\tmh-c09.inf",		
		"char\\tmABCD\\tmh-c10.inf",		
		"char\\tmABCD\\tmh-c11.inf",		
		"char\\tmABCD\\tmh-c12.inf",		
		"char\\tmABCD\\tmh-c13.inf"			

	},
	{
		"char\\tmABCD\\tmh-c01a.inf",
		"char\\tmABCD\\tmh-c02a.inf",
		"char\\tmABCD\\tmh-c03a.inf",
		"char\\tmABCD\\tmh-c04a.inf",
		"char\\tmABCD\\tmh-c05a.inf",
		"char\\tmABCD\\tmh-c06a.inf",
		"char\\tmABCD\\tmh-c07a.inf",
		"char\\tmABCD\\tmh-c08a.inf",
		"char\\tmABCD\\tmh-c09a.inf",
		"char\\tmABCD\\tmh-c10a.inf",
		"char\\tmABCD\\tmh-c11a.inf",
		"char\\tmABCD\\tmh-c12a.inf",
		"char\\tmABCD\\tmh-c13a.inf"

	},
	{
		"char\\tmABCD\\tmh_c01b.inf",
		"char\\tmABCD\\tmh_c02b.inf",
		"char\\tmABCD\\tmh_c03b.inf",
		"char\\tmABCD\\tmh_c04b.inf",
		"char\\tmABCD\\tmh_c05b.inf",
		"char\\tmABCD\\tmh_c06b.inf",
		"char\\tmABCD\\tmh_c07b.inf",
		"char\\tmABCD\\tmh_c08b.inf",
		"char\\tmABCD\\tmh_c09b.inf",
		"char\\tmABCD\\tmh_c10b.inf",
		"char\\tmABCD\\tmh_c11b.inf",
		"char\\tmABCD\\tmh_c12b.inf",
		"char\\tmABCD\\tmh_c13b.inf"
	},
/*
	{
		"char\\tmABCD\\tmh-c01c.inf",
		"char\\tmABCD\\tmh-c02c.inf",
		"char\\tmABCD\\tmh-c03c.inf"
	},
*/
	{
		"char\\tmABCD\\tmhC-01c.inf",
		"char\\tmABCD\\tmhC-02c.inf",
		"char\\tmABCD\\tmhC-03c.inf",
		"char\\tmABCD\\tmhC-04c.inf",
		"char\\tmABCD\\tmhC-05c.inf",
		"char\\tmABCD\\tmhC-06c.inf",
		"char\\tmABCD\\tmhC-07c.inf",
		"char\\tmABCD\\tmhC-08c.inf",
		"char\\tmABCD\\tmhC-09c.inf",
		"char\\tmABCD\\tmhC-10c.inf",
		"char\\tmABCD\\tmhC-11c.inf",
		"char\\tmABCD\\tmhC-12c.inf",
		"char\\tmABCD\\tmhC-13c.inf"

	},
	//////////// 얼큰 버전 //////////					
	{
		"char\\tmABCD\\tmh-c01b.inf",
		"char\\tmABCD\\tmh-c02b.inf",
		"char\\tmABCD\\tmh-c03b.inf",
		
		"char\\tmABCD\\tmh-c01b.inf",
		"char\\tmABCD\\tmh-c02b.inf",
		"char\\tmABCD\\tmh-c03b.inf",
		"char\\tmABCD\\tmh-c01b.inf",
		"char\\tmABCD\\tmh-c02b.inf",
		
		"char\\tmABCD\\tmh-c03b.inf",
		"char\\tmABCD\\tmh-c01b.inf",
		"char\\tmABCD\\tmh-c02b.inf",
		"char\\tmABCD\\tmh-c03b.inf",
		"char\\tmABCD\\tmh-c01b.inf"
	},
	{
		"char\\tmABCD\\tmh-c01t.inf",
		"char\\tmABCD\\tmh-c02t.inf",
		"char\\tmABCD\\tmh-c03t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\tmh-c01t.inf",
		"char\\tmABCD\\tmh-c02t.inf",
		"char\\tmABCD\\tmh-c03t.inf",
		"char\\tmABCD\\tmh-c01t.inf",
		"char\\tmABCD\\tmh-c02t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\tmh-c01t.inf",
		"char\\tmABCD\\tmh-c02t.inf",
		"char\\tmABCD\\tmh-c03t.inf",
		"char\\tmABCD\\tmh-c01t.inf",
		"char\\tmABCD\\tmh-c02t.inf"
	},

	// 박재원 - 2010 월드컵 이벤트 - 축구공 포션 추가(축구공 모델 이미지 추가 - 머리)
	{
		"char\\tmABCD\\TmhC-18t.inf",
		"char\\tmABCD\\TmhC-18t.inf",
		"char\\tmABCD\\TmhC-18t.inf",

		"char\\tmABCD\\TmhC-18t.inf",
		"char\\tmABCD\\TmhC-18t.inf",
		"char\\tmABCD\\TmhC-18t.inf",
		"char\\tmABCD\\TmhC-18t.inf",
		"char\\tmABCD\\TmhC-18t.inf",

		"char\\tmABCD\\TmhC-18t.inf",
		"char\\tmABCD\\TmhC-18t.inf",
		"char\\tmABCD\\TmhC-18t.inf",
		"char\\tmABCD\\TmhC-18t.inf",
		"char\\tmABCD\\TmhC-18t.inf"	
	}
};


char *szModel_KnightFaceName[FACE_MODEL_MAX][HEAREPOTION_MAX] = {	//
	{
		"char\\tmABCD\\Mmh-A01.inf",
		"char\\tmABCD\\Mmh-A02.inf",
		"char\\tmABCD\\Mmh-A03.inf",
		"char\\tmABCD\\Mmh-A04.inf",
		"char\\tmABCD\\Mmh-A05.inf",
		"char\\tmABCD\\Mmh-A06.inf",
		"char\\tmABCD\\Mmh-A07.inf",
		"char\\tmABCD\\Mmh-A08.inf",
		"char\\tmABCD\\Mmh-A09.inf",
		"char\\tmABCD\\Mmh-A10.inf",
		"char\\tmABCD\\Mmh-A11.inf",
		"char\\tmABCD\\Mmh-A12.inf",
		"char\\tmABCD\\Mmh-A13.inf"
	},
	{
		"char\\tmABCD\\Mmh-A01a.inf",
		"char\\tmABCD\\Mmh-A02a.inf",
		"char\\tmABCD\\Mmh-A03a.inf",
		"char\\tmABCD\\Mmh-A04a.inf",
		"char\\tmABCD\\Mmh-A05a.inf",
		"char\\tmABCD\\Mmh-A06a.inf",
		"char\\tmABCD\\Mmh-A07a.inf",
		"char\\tmABCD\\Mmh-A08a.inf",
		"char\\tmABCD\\Mmh-A09a.inf",
		"char\\tmABCD\\Mmh-A10a.inf",
		"char\\tmABCD\\Mmh-A11a.inf",
		"char\\tmABCD\\Mmh-A12a.inf",
		"char\\tmABCD\\Mmh-A13a.inf"
	},
	{
		"char\\tmABCD\\Mmh_A01b.inf",
		"char\\tmABCD\\Mmh_A02b.inf",
		"char\\tmABCD\\Mmh_A03b.inf",
		"char\\tmABCD\\Mmh_A04b.inf",
		"char\\tmABCD\\Mmh_A05b.inf",
		"char\\tmABCD\\Mmh_A06b.inf",
		"char\\tmABCD\\Mmh_A07b.inf",
		"char\\tmABCD\\Mmh_A08b.inf",
		"char\\tmABCD\\Mmh_A09b.inf",
		"char\\tmABCD\\Mmh_A10b.inf",
		"char\\tmABCD\\Mmh_A11b.inf",
		"char\\tmABCD\\Mmh_A12b.inf",
		"char\\tmABCD\\Mmh_A13b.inf"
	},
	{
		"char\\tmABCD\\Mmh-A01c.inf",
		"char\\tmABCD\\Mmh-A02c.inf",
		"char\\tmABCD\\Mmh-A03c.inf",
		"char\\tmABCD\\Mmh-A04c.inf",
		"char\\tmABCD\\Mmh-A05c.inf",
		"char\\tmABCD\\Mmh-A06c.inf",
		"char\\tmABCD\\Mmh-A07c.inf",
		"char\\tmABCD\\Mmh-A08c.inf",

		"char\\tmABCD\\Mmh-A09c.inf",
		"char\\tmABCD\\Mmh-A10c.inf",
		"char\\tmABCD\\Mmh-A11c.inf",
		"char\\tmABCD\\Mmh-A12c.inf",
		"char\\tmABCD\\Mmh-A13c.inf"
	},
	//////////// 얼큰 버전 //////////
	{
		"char\\tmABCD\\Mmh-A01b.inf",
		"char\\tmABCD\\Mmh-A02b.inf",
		"char\\tmABCD\\Mmh-A03b.inf",
		
		"char\\tmABCD\\Mmh-A01b.inf",
		"char\\tmABCD\\Mmh-A02b.inf",
		"char\\tmABCD\\Mmh-A03b.inf",
		"char\\tmABCD\\Mmh-A01b.inf",
		"char\\tmABCD\\Mmh-A02b.inf",
		
		"char\\tmABCD\\Mmh-A03b.inf",
		"char\\tmABCD\\Mmh-A01b.inf",
		"char\\tmABCD\\Mmh-A02b.inf",
		"char\\tmABCD\\Mmh-A03b.inf",
		"char\\tmABCD\\Mmh-A01b.inf"

	},
	{
		"char\\tmABCD\\Mmh-A01t.inf",
		"char\\tmABCD\\Mmh-A02t.inf",
		"char\\tmABCD\\Mmh-A03t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\Mmh-A01t.inf",
		"char\\tmABCD\\Mmh-A02t.inf",
		"char\\tmABCD\\Mmh-A03t.inf",
		"char\\tmABCD\\Mmh-A01t.inf",
		"char\\tmABCD\\Mmh-A02t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\Mmh-A01t.inf",
		"char\\tmABCD\\Mmh-A02t.inf",
		"char\\tmABCD\\Mmh-A03t.inf",
		"char\\tmABCD\\Mmh-A01t.inf",
		"char\\tmABCD\\Mmh-A02t.inf"		
	},
	
	// 박재원 - 2010 월드컵 이벤트 - 축구공 포션 추가(축구공 모델 이미지 추가 - 머리)
	{
		"char\\tmABCD\\MmhA-18t.inf",
		"char\\tmABCD\\MmhA-18t.inf",
		"char\\tmABCD\\MmhA-18t.inf",

		"char\\tmABCD\\MmhA-18t.inf",
		"char\\tmABCD\\MmhA-18t.inf",
		"char\\tmABCD\\MmhA-18t.inf",
		"char\\tmABCD\\MmhA-18t.inf",
		"char\\tmABCD\\MmhA-18t.inf",

		"char\\tmABCD\\MmhA-18t.inf",
		"char\\tmABCD\\MmhA-18t.inf",
		"char\\tmABCD\\MmhA-18t.inf",
		"char\\tmABCD\\MmhA-18t.inf",
		"char\\tmABCD\\MmhA-18t.inf"	
	}
};

char *szModel_AtalantaFaceName[FACE_MODEL_MAX][HEAREPOTION_MAX] = {			//아탈란타
	{
		"char\\tmABCD\\Mfh-B01.inf",
		"char\\tmABCD\\Mfh-B02.inf",
		"char\\tmABCD\\Mfh-B03.inf",
		"char\\tmABCD\\Mfh-B04.inf",
		"char\\tmABCD\\Mfh-B05.inf",
		"char\\tmABCD\\Mfh-B06.inf",
		"char\\tmABCD\\Mfh-B07.inf",
		"char\\tmABCD\\Mfh-B08.inf",

		"char\\tmABCD\\Mfh-B09.inf",
		"char\\tmABCD\\Mfh-B10.inf",
		"char\\tmABCD\\Mfh-B11.inf",
		"char\\tmABCD\\Mfh-B12.inf",
		"char\\tmABCD\\Mfh-B13.inf"
	},
	{
		"char\\tmABCD\\Mfh-B01a.inf",
		"char\\tmABCD\\Mfh-B02a.inf",
		"char\\tmABCD\\Mfh-B03a.inf",
		"char\\tmABCD\\Mfh-B04a.inf",
		"char\\tmABCD\\Mfh-B05a.inf",
		"char\\tmABCD\\Mfh-B06a.inf",
		"char\\tmABCD\\Mfh-B07a.inf",
		"char\\tmABCD\\Mfh-B08a.inf",

		"char\\tmABCD\\Mfh-B09a.inf",
		"char\\tmABCD\\Mfh-B10a.inf",
		"char\\tmABCD\\Mfh-B11a.inf",
		"char\\tmABCD\\Mfh-B12a.inf",
		"char\\tmABCD\\Mfh-B13a.inf"
	},
	{
		"char\\tmABCD\\Mfh_B01b.inf",
		"char\\tmABCD\\Mfh_B02b.inf",
		"char\\tmABCD\\Mfh_B03b.inf",
		"char\\tmABCD\\Mfh_B04b.inf",
		"char\\tmABCD\\Mfh_B05b.inf",
		"char\\tmABCD\\Mfh_B06b.inf",
		"char\\tmABCD\\Mfh_B07b.inf",
		"char\\tmABCD\\Mfh_B08b.inf",

		"char\\tmABCD\\Mfh_B09b.inf",
		"char\\tmABCD\\Mfh_B10b.inf",
		"char\\tmABCD\\Mfh_B11b.inf",
		"char\\tmABCD\\Mfh_B12b.inf",
		"char\\tmABCD\\Mfh_B13b.inf"
	},
	{
		"char\\tmABCD\\Mfh-B01c.inf",
		"char\\tmABCD\\Mfh-B02c.inf",
		"char\\tmABCD\\Mfh-B03c.inf",
		"char\\tmABCD\\Mfh-B04c.inf",
		"char\\tmABCD\\Mfh-B05c.inf",
		"char\\tmABCD\\Mfh-B06c.inf",
		"char\\tmABCD\\Mfh-B07c.inf",
		"char\\tmABCD\\Mfh-B08c.inf",

		"char\\tmABCD\\Mfh-B09c.inf",
		"char\\tmABCD\\Mfh-B10c.inf",
		"char\\tmABCD\\Mfh-B11c.inf",
		"char\\tmABCD\\Mfh-B12c.inf",
		"char\\tmABCD\\Mfh-B13c.inf"
	
	},
	//////////// 얼큰 버전 //////////
	{
		"char\\tmABCD\\Mfh-B01t.inf",
		"char\\tmABCD\\Mfh-B02t.inf",
		"char\\tmABCD\\Mfh-B03t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.	
		"char\\tmABCD\\Mfh-B01t.inf",
		"char\\tmABCD\\Mfh-B02t.inf",
		"char\\tmABCD\\Mfh-B03t.inf",
		"char\\tmABCD\\Mfh-B01t.inf",
		"char\\tmABCD\\Mfh-B02t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\Mfh-B01t.inf",
		"char\\tmABCD\\Mfh-B02t.inf",
		"char\\tmABCD\\Mfh-B03t.inf",
		"char\\tmABCD\\Mfh-B01t.inf",
		"char\\tmABCD\\Mfh-B02t.inf"
	},
	{
		"char\\tmABCD\\Mfh-B01t.inf",
		"char\\tmABCD\\Mfh-B02t.inf",
		"char\\tmABCD\\Mfh-B03t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\Mfh-B01t.inf",
		"char\\tmABCD\\Mfh-B02t.inf",
		"char\\tmABCD\\Mfh-B03t.inf",
		"char\\tmABCD\\Mfh-B01t.inf",
		"char\\tmABCD\\Mfh-B02t.inf",		
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\Mfh-B01t.inf",
		"char\\tmABCD\\Mfh-B02t.inf",
		"char\\tmABCD\\Mfh-B03t.inf",
		"char\\tmABCD\\Mfh-B01t.inf",
		"char\\tmABCD\\Mfh-B02t.inf",	
	},

	// 박재원 - 2010 월드컵 이벤트 - 축구공 포션 추가(축구공 모델 이미지 추가 - 머리)
	{
		"char\\tmABCD\\MfhB-18t.inf",
		"char\\tmABCD\\MfhB-18t.inf",
		"char\\tmABCD\\MfhB-18t.inf",

		"char\\tmABCD\\MfhB-18t.inf",
		"char\\tmABCD\\MfhB-18t.inf",
		"char\\tmABCD\\MfhB-18t.inf",
		"char\\tmABCD\\MfhB-18t.inf",
		"char\\tmABCD\\MfhB-18t.inf",

		"char\\tmABCD\\MfhB-18t.inf",
		"char\\tmABCD\\MfhB-18t.inf",
		"char\\tmABCD\\MfhB-18t.inf",
		"char\\tmABCD\\MfhB-18t.inf",
		"char\\tmABCD\\MfhB-18t.inf"	
	}
};


char *szModel_PristessFaceName[FACE_MODEL_MAX][HEAREPOTION_MAX] = {				//프리스티스 
	{
		"char\\tmABCD\\Mfh-C01.inf",
		"char\\tmABCD\\Mfh-C02.inf",
		"char\\tmABCD\\Mfh-C03.inf",
		"char\\tmABCD\\Mfh-C04.inf",
		"char\\tmABCD\\Mfh-C05.inf",
		"char\\tmABCD\\Mfh-C06.inf",
		"char\\tmABCD\\Mfh-C07.inf",
		"char\\tmABCD\\Mfh-C08.inf",

		"char\\tmABCD\\Mfh-C09.inf",
		"char\\tmABCD\\Mfh-C10.inf",
		"char\\tmABCD\\Mfh-C11.inf",
		"char\\tmABCD\\Mfh-C12.inf",
		"char\\tmABCD\\Mfh-C13.inf"
	},
	{
		"char\\tmABCD\\Mfh-C01a.inf",
		"char\\tmABCD\\Mfh-C02a.inf",
		"char\\tmABCD\\Mfh-C03a.inf",
		"char\\tmABCD\\Mfh-C04a.inf",
		"char\\tmABCD\\Mfh-C05a.inf",
		"char\\tmABCD\\Mfh-C06a.inf",
		"char\\tmABCD\\Mfh-C07a.inf",
		"char\\tmABCD\\Mfh-C08a.inf",

		"char\\tmABCD\\Mfh-C09a.inf",
		"char\\tmABCD\\Mfh-C10a.inf",
		"char\\tmABCD\\Mfh-C11a.inf",
		"char\\tmABCD\\Mfh-C12a.inf",
		"char\\tmABCD\\Mfh-C13a.inf"
	},
	{
		"char\\tmABCD\\Mfh_C01b.inf",
		"char\\tmABCD\\Mfh_C02b.inf",
		"char\\tmABCD\\Mfh_C03b.inf",
		"char\\tmABCD\\Mfh_C04b.inf",
		"char\\tmABCD\\Mfh_C05b.inf",
		"char\\tmABCD\\Mfh_C06b.inf",
		"char\\tmABCD\\Mfh_C07b.inf",
		"char\\tmABCD\\Mfh_C08b.inf",

		"char\\tmABCD\\Mfh_C09b.inf",
		"char\\tmABCD\\Mfh_C10b.inf",
		"char\\tmABCD\\Mfh_C11b.inf",
		"char\\tmABCD\\Mfh_C12b.inf",
		"char\\tmABCD\\Mfh_C13b.inf"
	},
	{
		"char\\tmABCD\\Mfh-C01c.inf",
		"char\\tmABCD\\Mfh-C02c.inf",
		"char\\tmABCD\\Mfh-C03c.inf",
		"char\\tmABCD\\Mfh-C04c.inf",
		"char\\tmABCD\\Mfh-C05c.inf",
		"char\\tmABCD\\Mfh-C06c.inf",
		"char\\tmABCD\\Mfh-C07c.inf",
		"char\\tmABCD\\Mfh-C08c.inf",

		"char\\tmABCD\\Mfh-C09c.inf",
		"char\\tmABCD\\Mfh-C10c.inf",
		"char\\tmABCD\\Mfh-C11c.inf",
		"char\\tmABCD\\Mfh-C12c.inf",
		"char\\tmABCD\\Mfh-C13c.inf"

	},
	//////////// 얼큰 버전 //////////
	{
		"char\\tmABCD\\Mfh-C01t.inf",
		"char\\tmABCD\\Mfh-C02t.inf",
		"char\\tmABCD\\Mfh-C03t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.
		"char\\tmABCD\\Mfh-C01t.inf",
		"char\\tmABCD\\Mfh-C02t.inf",
		"char\\tmABCD\\Mfh-C03t.inf",
		"char\\tmABCD\\Mfh-C01t.inf",
		"char\\tmABCD\\Mfh-C02t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.	
		"char\\tmABCD\\Mfh-C01t.inf",
		"char\\tmABCD\\Mfh-C02t.inf",
		"char\\tmABCD\\Mfh-C03t.inf",
		"char\\tmABCD\\Mfh-C01t.inf",
		"char\\tmABCD\\Mfh-C02t.inf"
		
	},
	{
		"char\\tmABCD\\Mfh-C01t.inf",
		"char\\tmABCD\\Mfh-C02t.inf",
		"char\\tmABCD\\Mfh-C03t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.	
		"char\\tmABCD\\Mfh-C01t.inf",
		"char\\tmABCD\\Mfh-C02t.inf",
		"char\\tmABCD\\Mfh-C03t.inf",
		"char\\tmABCD\\Mfh-C01t.inf",
		"char\\tmABCD\\Mfh-C02t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.	
		"char\\tmABCD\\Mfh-C01t.inf",
		"char\\tmABCD\\Mfh-C02t.inf",
		"char\\tmABCD\\Mfh-C03t.inf",
		"char\\tmABCD\\Mfh-C01t.inf",
		"char\\tmABCD\\Mfh-C02t.inf"
	},

	// 박재원 - 2010 월드컵 이벤트 - 축구공 포션 추가(축구공 모델 이미지 추가 - 머리)
	{
		"char\\tmABCD\\MfhC-18t.inf",
		"char\\tmABCD\\MfhC-18t.inf",
		"char\\tmABCD\\MfhC-18t.inf",

		"char\\tmABCD\\MfhC-18t.inf",
		"char\\tmABCD\\MfhC-18t.inf",
		"char\\tmABCD\\MfhC-18t.inf",
		"char\\tmABCD\\MfhC-18t.inf",
		"char\\tmABCD\\MfhC-18t.inf",

		"char\\tmABCD\\MfhC-18t.inf",
		"char\\tmABCD\\MfhC-18t.inf",
		"char\\tmABCD\\MfhC-18t.inf",
		"char\\tmABCD\\MfhC-18t.inf",
		"char\\tmABCD\\MfhC-18t.inf"	
	}	
};

char *szModel_Magican[FACE_MODEL_MAX][HEAREPOTION_MAX] = {			//메지션
	{	
		"char\\tmABCD\\Mmh-D01.inf",
		"char\\tmABCD\\Mmh-D02.inf",
		"char\\tmABCD\\Mmh-D03.inf",
		"char\\tmABCD\\Mmh-D04.inf",
		"char\\tmABCD\\Mmh-D05.inf",
		"char\\tmABCD\\Mmh-D06.inf",
		"char\\tmABCD\\Mmh-D07.inf",
		"char\\tmABCD\\Mmh-D08.inf",

		"char\\tmABCD\\Mmh-D09.inf",
		"char\\tmABCD\\Mmh-D10.inf",
		"char\\tmABCD\\Mmh-D11.inf",
		"char\\tmABCD\\Mmh-D12.inf",
		"char\\tmABCD\\Mmh-D13.inf"
	},
	{
		"char\\tmABCD\\Mmh-D01a.inf",
		"char\\tmABCD\\Mmh-D02a.inf",
		"char\\tmABCD\\Mmh-D03a.inf",
		"char\\tmABCD\\Mmh-D04a.inf",
		"char\\tmABCD\\Mmh-D05a.inf",
		"char\\tmABCD\\Mmh-D06a.inf",
		"char\\tmABCD\\Mmh-D07a.inf",
		"char\\tmABCD\\Mmh-D08a.inf",

		"char\\tmABCD\\Mmh-D09a.inf",
		"char\\tmABCD\\Mmh-D10a.inf",
		"char\\tmABCD\\Mmh-D11a.inf",
		"char\\tmABCD\\Mmh-D12a.inf",
		"char\\tmABCD\\Mmh-D13a.inf"
	},
	{
		"char\\tmABCD\\Mmh_D01b.inf",
		"char\\tmABCD\\Mmh_D02b.inf",
		"char\\tmABCD\\Mmh_D03b.inf",
		"char\\tmABCD\\Mmh_D04b.inf",
		"char\\tmABCD\\Mmh_D05b.inf",
		"char\\tmABCD\\Mmh_D06b.inf",
		"char\\tmABCD\\Mmh_D07b.inf",
		"char\\tmABCD\\Mmh_D08b.inf",

		"char\\tmABCD\\Mmh_D09b.inf",
		"char\\tmABCD\\Mmh_D10b.inf",
		"char\\tmABCD\\Mmh_D11b.inf",
		"char\\tmABCD\\Mmh_D12b.inf",
		"char\\tmABCD\\Mmh_D13b.inf"
	},
	{
		"char\\tmABCD\\Mmh-D01c.inf",
		"char\\tmABCD\\Mmh-D02c.inf",
		"char\\tmABCD\\Mmh-D03c.inf",
		"char\\tmABCD\\Mmh-D04c.inf",
		"char\\tmABCD\\Mmh-D05c.inf",
		"char\\tmABCD\\Mmh-D06c.inf",
		"char\\tmABCD\\Mmh-D07c.inf",
		"char\\tmABCD\\Mmh-D08c.inf",

		"char\\tmABCD\\Mmh-D09c.inf",
		"char\\tmABCD\\Mmh-D10c.inf",
		"char\\tmABCD\\Mmh-D11c.inf",
		"char\\tmABCD\\Mmh-D12c.inf",
		"char\\tmABCD\\Mmh-D13c.inf"
	},
	//////////// 얼큰 버전 //////////
	{	
		"char\\tmABCD\\Mmh-D01b.inf",
		"char\\tmABCD\\Mmh-D02b.inf",
		"char\\tmABCD\\Mmh-D03b.inf",
		
		"char\\tmABCD\\Mmh-D01b.inf",
		"char\\tmABCD\\Mmh-D02b.inf",
		"char\\tmABCD\\Mmh-D03b.inf",
		"char\\tmABCD\\Mmh-D01b.inf",
		"char\\tmABCD\\Mmh-D02b.inf",
		
		"char\\tmABCD\\Mmh-D03b.inf",
		"char\\tmABCD\\Mmh-D01b.inf",
		"char\\tmABCD\\Mmh-D02b.inf",
		"char\\tmABCD\\Mmh-D03b.inf",
		"char\\tmABCD\\Mmh-D01b.inf"

	},
	{
		"char\\tmABCD\\Mmh-D01t.inf",
		"char\\tmABCD\\Mmh-D02t.inf",
		"char\\tmABCD\\Mmh-D03t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.	
		"char\\tmABCD\\Mmh-D01t.inf",
		"char\\tmABCD\\Mmh-D02t.inf",
		"char\\tmABCD\\Mmh-D03t.inf",
		"char\\tmABCD\\Mmh-D01t.inf",
		"char\\tmABCD\\Mmh-D02t.inf",
		//헤어틴트포션때문에 가데이터로 채워 넣는다.. 얼큰 버젼 사용시 바꿔줘야함.	
		"char\\tmABCD\\Mmh-D01t.inf",
		"char\\tmABCD\\Mmh-D02t.inf",
		"char\\tmABCD\\Mmh-D03t.inf",
		"char\\tmABCD\\Mmh-D01t.inf",
		"char\\tmABCD\\Mmh-D02t.inf"
	},

	// 박재원 - 2010 월드컵 이벤트 - 축구공 포션 추가(축구공 모델 이미지 추가 - 머리)
	{
		"char\\tmABCD\\MmhD-18t.inf",
		"char\\tmABCD\\MmhD-18t.inf",
		"char\\tmABCD\\MmhD-18t.inf",

		"char\\tmABCD\\MmhD-18t.inf",
		"char\\tmABCD\\MmhD-18t.inf",
		"char\\tmABCD\\MmhD-18t.inf",
		"char\\tmABCD\\MmhD-18t.inf",
		"char\\tmABCD\\MmhD-18t.inf",

		"char\\tmABCD\\MmhD-18t.inf",
		"char\\tmABCD\\MmhD-18t.inf",
		"char\\tmABCD\\MmhD-18t.inf",
		"char\\tmABCD\\MmhD-18t.inf",
		"char\\tmABCD\\MmhD-18t.inf"	
	}
};

// Assassin (JOB_CODE 9): heads Tfh-E01..13; a/b/c/d/t only for 01..03 (the rest keep the plain head)
char *szModel_AssassinFaceName[FACE_MODEL_MAX][HEAREPOTION_MAX] = {
	{
		"char\\tmABCD\\Tfh-E01.inf",
		"char\\tmABCD\\Tfh-E02.inf",
		"char\\tmABCD\\Tfh-E03.inf",
		"char\\tmABCD\\Tfh-E04.inf",
		"char\\tmABCD\\Tfh-E05.inf",
		"char\\tmABCD\\Tfh-E06.inf",
		"char\\tmABCD\\Tfh-E07.inf",
		"char\\tmABCD\\Tfh-E08.inf",
		"char\\tmABCD\\Tfh-E09.inf",
		"char\\tmABCD\\Tfh-E10.inf",
		"char\\tmABCD\\Tfh-E11.inf",
		"char\\tmABCD\\Tfh-E12.inf",
		"char\\tmABCD\\Tfh-E13.inf"
	},
	{
		"char\\tmABCD\\Tfh-E01a.inf",
		"char\\tmABCD\\Tfh-E02a.inf",
		"char\\tmABCD\\Tfh-E03a.inf",
		"char\\tmABCD\\Tfh-E04.inf",
		"char\\tmABCD\\Tfh-E05.inf",
		"char\\tmABCD\\Tfh-E06.inf",
		"char\\tmABCD\\Tfh-E07.inf",
		"char\\tmABCD\\Tfh-E08.inf",
		"char\\tmABCD\\Tfh-E09.inf",
		"char\\tmABCD\\Tfh-E10.inf",
		"char\\tmABCD\\Tfh-E11.inf",
		"char\\tmABCD\\Tfh-E12.inf",
		"char\\tmABCD\\Tfh-E13.inf"
	},
	{
		"char\\tmABCD\\Tfh-E01b.inf",
		"char\\tmABCD\\Tfh-E02b.inf",
		"char\\tmABCD\\Tfh-E03b.inf",
		"char\\tmABCD\\Tfh-E04.inf",
		"char\\tmABCD\\Tfh-E05.inf",
		"char\\tmABCD\\Tfh-E06.inf",
		"char\\tmABCD\\Tfh-E07.inf",
		"char\\tmABCD\\Tfh-E08.inf",
		"char\\tmABCD\\Tfh-E09.inf",
		"char\\tmABCD\\Tfh-E10.inf",
		"char\\tmABCD\\Tfh-E11.inf",
		"char\\tmABCD\\Tfh-E12.inf",
		"char\\tmABCD\\Tfh-E13.inf"
	},
	{
		"char\\tmABCD\\Tfh-E01c.inf",
		"char\\tmABCD\\Tfh-E02c.inf",
		"char\\tmABCD\\Tfh-E03c.inf",
		"char\\tmABCD\\Tfh-E04.inf",
		"char\\tmABCD\\Tfh-E05.inf",
		"char\\tmABCD\\Tfh-E06.inf",
		"char\\tmABCD\\Tfh-E07.inf",
		"char\\tmABCD\\Tfh-E08.inf",
		"char\\tmABCD\\Tfh-E09.inf",
		"char\\tmABCD\\Tfh-E10.inf",
		"char\\tmABCD\\Tfh-E11.inf",
		"char\\tmABCD\\Tfh-E12.inf",
		"char\\tmABCD\\Tfh-E13.inf"
	},
	{
		"char\\tmABCD\\Tfh-E01t.inf",
		"char\\tmABCD\\Tfh-E02t.inf",
		"char\\tmABCD\\Tfh-E03t.inf",
		"char\\tmABCD\\Tfh-E01t.inf",
		"char\\tmABCD\\Tfh-E02t.inf",
		"char\\tmABCD\\Tfh-E03t.inf",
		"char\\tmABCD\\Tfh-E01t.inf",
		"char\\tmABCD\\Tfh-E02t.inf",
		"char\\tmABCD\\Tfh-E03t.inf",
		"char\\tmABCD\\Tfh-E01t.inf",
		"char\\tmABCD\\Tfh-E02t.inf",
		"char\\tmABCD\\Tfh-E03t.inf",
		"char\\tmABCD\\Tfh-E01t.inf"
	},
	{
		"char\\tmABCD\\Tfh-E01t.inf",
		"char\\tmABCD\\Tfh-E02t.inf",
		"char\\tmABCD\\Tfh-E03t.inf",
		"char\\tmABCD\\Tfh-E01t.inf",
		"char\\tmABCD\\Tfh-E02t.inf",
		"char\\tmABCD\\Tfh-E03t.inf",
		"char\\tmABCD\\Tfh-E01t.inf",
		"char\\tmABCD\\Tfh-E02t.inf",
		"char\\tmABCD\\Tfh-E03t.inf",
		"char\\tmABCD\\Tfh-E01t.inf",
		"char\\tmABCD\\Tfh-E02t.inf",
		"char\\tmABCD\\Tfh-E03t.inf",
		"char\\tmABCD\\Tfh-E01t.inf"
	},
	{
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf",
		"char\\tmABCD\\Tfh-E18t.inf"
	}
};

// Shaman (JOB_CODE 10): heads Mmh-E01..13; a/b/c/d/t only for 01..03 (the rest keep the plain head)
char *szModel_ShamanFaceName[FACE_MODEL_MAX][HEAREPOTION_MAX] = {
	{
		"char\\tmABCD\\Mmh-E01.inf",
		"char\\tmABCD\\Mmh-E02.inf",
		"char\\tmABCD\\Mmh-E03.inf",
		"char\\tmABCD\\Mmh-E04.inf",
		"char\\tmABCD\\Mmh-E05.inf",
		"char\\tmABCD\\Mmh-E06.inf",
		"char\\tmABCD\\Mmh-E07.inf",
		"char\\tmABCD\\Mmh-E08.inf",
		"char\\tmABCD\\Mmh-E09.inf",
		"char\\tmABCD\\Mmh-E10.inf",
		"char\\tmABCD\\Mmh-E11.inf",
		"char\\tmABCD\\Mmh-E12.inf",
		"char\\tmABCD\\Mmh-E13.inf"
	},
	{
		"char\\tmABCD\\Mmh-E01a.inf",
		"char\\tmABCD\\Mmh-E02a.inf",
		"char\\tmABCD\\Mmh-E03a.inf",
		"char\\tmABCD\\Mmh-E04.inf",
		"char\\tmABCD\\Mmh-E05.inf",
		"char\\tmABCD\\Mmh-E06.inf",
		"char\\tmABCD\\Mmh-E07.inf",
		"char\\tmABCD\\Mmh-E08.inf",
		"char\\tmABCD\\Mmh-E09.inf",
		"char\\tmABCD\\Mmh-E10.inf",
		"char\\tmABCD\\Mmh-E11.inf",
		"char\\tmABCD\\Mmh-E12.inf",
		"char\\tmABCD\\Mmh-E13.inf"
	},
	{
		"char\\tmABCD\\Mmh-E01b.inf",
		"char\\tmABCD\\Mmh-E02b.inf",
		"char\\tmABCD\\Mmh-E03b.inf",
		"char\\tmABCD\\Mmh-E04.inf",
		"char\\tmABCD\\Mmh-E05.inf",
		"char\\tmABCD\\Mmh-E06.inf",
		"char\\tmABCD\\Mmh-E07.inf",
		"char\\tmABCD\\Mmh-E08.inf",
		"char\\tmABCD\\Mmh-E09.inf",
		"char\\tmABCD\\Mmh-E10.inf",
		"char\\tmABCD\\Mmh-E11.inf",
		"char\\tmABCD\\Mmh-E12.inf",
		"char\\tmABCD\\Mmh-E13.inf"
	},
	{
		"char\\tmABCD\\Mmh-E01c.inf",
		"char\\tmABCD\\Mmh-E02c.inf",
		"char\\tmABCD\\Mmh-E03c.inf",
		"char\\tmABCD\\Mmh-E04.inf",
		"char\\tmABCD\\Mmh-E05.inf",
		"char\\tmABCD\\Mmh-E06.inf",
		"char\\tmABCD\\Mmh-E07.inf",
		"char\\tmABCD\\Mmh-E08.inf",
		"char\\tmABCD\\Mmh-E09.inf",
		"char\\tmABCD\\Mmh-E10.inf",
		"char\\tmABCD\\Mmh-E11.inf",
		"char\\tmABCD\\Mmh-E12.inf",
		"char\\tmABCD\\Mmh-E13.inf"
	},
	{
		"char\\tmABCD\\Mmh-E01t.inf",
		"char\\tmABCD\\Mmh-E02t.inf",
		"char\\tmABCD\\Mmh-E03t.inf",
		"char\\tmABCD\\Mmh-E01t.inf",
		"char\\tmABCD\\Mmh-E02t.inf",
		"char\\tmABCD\\Mmh-E03t.inf",
		"char\\tmABCD\\Mmh-E01t.inf",
		"char\\tmABCD\\Mmh-E02t.inf",
		"char\\tmABCD\\Mmh-E03t.inf",
		"char\\tmABCD\\Mmh-E01t.inf",
		"char\\tmABCD\\Mmh-E02t.inf",
		"char\\tmABCD\\Mmh-E03t.inf",
		"char\\tmABCD\\Mmh-E01t.inf"
	},
	{
		"char\\tmABCD\\Mmh-E01t.inf",
		"char\\tmABCD\\Mmh-E02t.inf",
		"char\\tmABCD\\Mmh-E03t.inf",
		"char\\tmABCD\\Mmh-E01t.inf",
		"char\\tmABCD\\Mmh-E02t.inf",
		"char\\tmABCD\\Mmh-E03t.inf",
		"char\\tmABCD\\Mmh-E01t.inf",
		"char\\tmABCD\\Mmh-E02t.inf",
		"char\\tmABCD\\Mmh-E03t.inf",
		"char\\tmABCD\\Mmh-E01t.inf",
		"char\\tmABCD\\Mmh-E02t.inf",
		"char\\tmABCD\\Mmh-E03t.inf",
		"char\\tmABCD\\Mmh-E01t.inf"
	},
	{
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf",
		"char\\tmABCD\\Mmh-E18t.inf"
	}
};

// Martial Artist (JOB_CODE 11): heads tfh-f01..13; a/b/c/d/t only for 01..03 (the rest keep the plain head)
char *szModel_MartialFaceName[FACE_MODEL_MAX][HEAREPOTION_MAX] = {
	{
		"char\\tmABCD\\tfh-f01.inf",
		"char\\tmABCD\\tfh-f02.inf",
		"char\\tmABCD\\tfh-f03.inf",
		"char\\tmABCD\\tfh-f04.inf",
		"char\\tmABCD\\tfh-f05.inf",
		"char\\tmABCD\\tfh-f06.inf",
		"char\\tmABCD\\tfh-f07.inf",
		"char\\tmABCD\\tfh-f08.inf",
		"char\\tmABCD\\tfh-f09.inf",
		"char\\tmABCD\\tfh-f10.inf",
		"char\\tmABCD\\tfh-f11.inf",
		"char\\tmABCD\\tfh-f12.inf",
		"char\\tmABCD\\tfh-f13.inf"
	},
	{
		"char\\tmABCD\\tfh-f01a.inf",
		"char\\tmABCD\\tfh-f02a.inf",
		"char\\tmABCD\\tfh-f03a.inf",
		"char\\tmABCD\\tfh-f04.inf",
		"char\\tmABCD\\tfh-f05.inf",
		"char\\tmABCD\\tfh-f06.inf",
		"char\\tmABCD\\tfh-f07.inf",
		"char\\tmABCD\\tfh-f08.inf",
		"char\\tmABCD\\tfh-f09.inf",
		"char\\tmABCD\\tfh-f10.inf",
		"char\\tmABCD\\tfh-f11.inf",
		"char\\tmABCD\\tfh-f12.inf",
		"char\\tmABCD\\tfh-f13.inf"
	},
	{
		"char\\tmABCD\\tfh-f01b.inf",
		"char\\tmABCD\\tfh-f02b.inf",
		"char\\tmABCD\\tfh-f03b.inf",
		"char\\tmABCD\\tfh-f04.inf",
		"char\\tmABCD\\tfh-f05.inf",
		"char\\tmABCD\\tfh-f06.inf",
		"char\\tmABCD\\tfh-f07.inf",
		"char\\tmABCD\\tfh-f08.inf",
		"char\\tmABCD\\tfh-f09.inf",
		"char\\tmABCD\\tfh-f10.inf",
		"char\\tmABCD\\tfh-f11.inf",
		"char\\tmABCD\\tfh-f12.inf",
		"char\\tmABCD\\tfh-f13.inf"
	},
	{
		"char\\tmABCD\\tfh-f01c.inf",
		"char\\tmABCD\\tfh-f02c.inf",
		"char\\tmABCD\\tfh-f03c.inf",
		"char\\tmABCD\\tfh-f04.inf",
		"char\\tmABCD\\tfh-f05.inf",
		"char\\tmABCD\\tfh-f06.inf",
		"char\\tmABCD\\tfh-f07.inf",
		"char\\tmABCD\\tfh-f08.inf",
		"char\\tmABCD\\tfh-f09.inf",
		"char\\tmABCD\\tfh-f10.inf",
		"char\\tmABCD\\tfh-f11.inf",
		"char\\tmABCD\\tfh-f12.inf",
		"char\\tmABCD\\tfh-f13.inf"
	},
	{
		"char\\tmABCD\\tfh-f01t.inf",
		"char\\tmABCD\\tfh-f02t.inf",
		"char\\tmABCD\\tfh-f03t.inf",
		"char\\tmABCD\\tfh-f01t.inf",
		"char\\tmABCD\\tfh-f02t.inf",
		"char\\tmABCD\\tfh-f03t.inf",
		"char\\tmABCD\\tfh-f01t.inf",
		"char\\tmABCD\\tfh-f02t.inf",
		"char\\tmABCD\\tfh-f03t.inf",
		"char\\tmABCD\\tfh-f01t.inf",
		"char\\tmABCD\\tfh-f02t.inf",
		"char\\tmABCD\\tfh-f03t.inf",
		"char\\tmABCD\\tfh-f01t.inf"
	},
	{
		"char\\tmABCD\\tfh-f01t.inf",
		"char\\tmABCD\\tfh-f02t.inf",
		"char\\tmABCD\\tfh-f03t.inf",
		"char\\tmABCD\\tfh-f01t.inf",
		"char\\tmABCD\\tfh-f02t.inf",
		"char\\tmABCD\\tfh-f03t.inf",
		"char\\tmABCD\\tfh-f01t.inf",
		"char\\tmABCD\\tfh-f02t.inf",
		"char\\tmABCD\\tfh-f03t.inf",
		"char\\tmABCD\\tfh-f01t.inf",
		"char\\tmABCD\\tfh-f02t.inf",
		"char\\tmABCD\\tfh-f03t.inf",
		"char\\tmABCD\\tfh-f01t.inf"
	},
	{
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf",
		"char\\tmABCD\\tfh-f18t.inf"
	}
};

#define	ARMOR_JOB_ROWS		11	// armour rows (playsub.cpp ArmorJobNum): 8 classes + the Assassin, the Shaman, the Martial Artist
#define	ARMOR_FILE_MAX		54 // 54

//일반 옷 모델 목록
char *szArmorNumFile[ARMOR_JOB_ROWS][ARMOR_FILE_MAX] = {
	{ "A001.ini" , "A001.ini",  "A002.ini",  "A003.ini",  "A004.ini",  "A005.ini",  "A006.ini",  "A007.ini", "A008.ini", "A009.ini", "A010.ini", 
	  "A011.ini" , "A012.ini",  "A013.ini",  "A014.ini",  "A015.ini",  "A016.ini",  "A017.ini",  "A018.ini", "A019.ini", "A020.ini", "A021.ini",  
	  "A001.ini" , "A022.ini",  "A023.ini",  "A024.ini",  "A001.ini",  "A001.ini",  "A001.ini",  "A001.ini",
	  "Ca001.ini", "Ca001.ini", "Ca001.ini", "Ca001.ini", "Ca002.ini", "Ca002.ini", "Ca002.ini", "Ca002.ini",
	  "Ca003.ini", "Ca003.ini", "Ca003.ini", "Ca003.ini", "Ca004.ini", "Ca004.ini", "Ca004.ini", "Ca004.ini",
	  "Ca005.ini", "Ca006.ini", "Ca007.ini", "Ca008.ini", "Ca009.ini", "Ca010.ini", "Ca011.ini", "Ca012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가

	},

	{ "B001.ini" , "B001.ini",  "B002.ini",  "B003.ini",  "B004.ini" , "B005.ini",  "B006.ini",  "B007.ini", "B008.ini", "B009.ini", "B010.ini" , 
	  "B011.ini" , "B012.ini",  "B013.ini",  "B014.ini",  "B015.ini" , "B016.ini",  "B017.ini",  "B018.ini", "B019.ini", "B020.ini", "B021.ini" ,
	  "B001.ini" , "B022.ini" , "B023.ini",  "B024.ini",  "B001.ini",  "B001.ini" , "B001.ini" , "B001.ini",
	  "Cb001.ini", "Cb001.ini", "Cb001.ini", "Cb001.ini", "Cb002.ini", "Cb002.ini", "Cb002.ini", "Cb002.ini",	
	  "Cb003.ini", "Cb003.ini", "Cb003.ini", "Cb003.ini", "Cb004.ini", "Cb004.ini", "Cb004.ini", "Cb004.ini",
	  "Cb005.ini", "Cb006.ini", "Cb007.ini", "Cb008.ini", "Cb009.ini", "Cb010.ini", "Cb011.ini", "Cb012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	
	{ "C001.ini" , "C001.ini",  "C002.ini",  "C003.ini",  "C004.ini",  "C005.ini",  "C006.ini",  "C007.ini",  "C008.ini", "C009.ini", "C010.ini",
	  "C011.ini" , "C012.ini",  "C013.ini",  "C014.ini",  "C015.ini",  "C016.ini",  "C017.ini",  "C018.ini",  "C019.ini", "C020.ini", "C021.ini",
	  "C001.ini" , "C022.ini",  "C023.ini",  "C024.ini",  "C001.ini",  "C001.ini",  "C001.ini",  "C001.ini",
	  "Cc001.ini", "Cc001.ini", "Cc001.ini", "Cc001.ini", "Cc002.ini", "Cc002.ini", "Cc002.ini", "Cc002.ini",
	  "Cc003.ini", "Cc003.ini", "Cc003.ini", "Cc003.ini", "Cc004.ini", "Cc004.ini", "Cc004.ini", "Cc004.ini",
	  "Cc005.ini", "Cc006.ini", "Cc007.ini", "Cc008.ini", "Cc009.ini", "Cc010.ini", "Cc011.ini", "Cc012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	{ "D001.ini" , "D001.ini",  "D002.ini",  "D003.ini",  "D004.ini",  "D005.ini",  "D006.ini",  "D007.ini", "D008.ini", "D009.ini", "D010.ini", 
	  "D011.ini" , "D012.ini",  "D013.ini",  "D014.ini",  "D015.ini",  "D016.ini",  "D017.ini",  "D018.ini", "D019.ini", "D020.ini", "D021.ini",
	  "D001.ini" , "D022.ini",  "D023.ini",  "D024.ini",  "D001.ini",  "D001.ini",  "D001.ini",  "D001.ini",
	  "Cd001.ini", "Cd001.ini", "Cd001.ini", "Cd001.ini", "Cd002.ini", "Cd002.ini", "Cd002.ini", "Cd002.ini",
	  "Cd003.ini", "Cd003.ini", "Cd003.ini", "Cd003.ini", "Cd004.ini", "Cd004.ini", "Cd004.ini", "Cd004.ini",
	  "Cd005.ini", "Cd006.ini", "Cd007.ini", "Cd008.ini", "Cd009.ini", "Cd010.ini", "Cd011.ini", "Cd012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	{ "mA001.ini",  "mA001.ini",  "mA002.ini",  "mA003.ini",   "mA004.ini",  "mA005.ini",  "mA006.ini",  "mA007.ini", "mA008.ini", "mA009.ini", "mA010.ini", 
	  "mA011.ini",  "mA012.ini",  "mA013.ini",  "mA014.ini",   "mA015.ini",  "mA016.ini",  "mA017.ini",  "mA018.ini", "mA019.ini", "mA020.ini", "mA021.ini",
	  "mA001.ini",  "mA022.ini",  "mA023.ini",  "mA024.ini",  "mA001.ini",   "mA001.ini",  "mA001.ini",  "mA001.ini",
	  "Cma001.ini", "Cma001.ini", "Cma001.ini", "Cma001.ini",  "Cma002.ini", "Cma002.ini", "Cma002.ini", "Cma002.ini",
	  "Cma003.ini", "Cma003.ini", "Cma003.ini", "Cma003.ini",  "Cma004.ini", "Cma004.ini", "Cma004.ini", "Cma004.ini",
	  "Cma005.ini", "Cma006.ini", "Cma007.ini", "Cma008.ini", "Cma009.ini", "Cma010.ini", "Cma011.ini", "Cma012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},
	
	{ "mB001.ini",  "mB001.ini",  "mB002.ini",  "mB003.ini",  "mB004.ini",  "mB005.ini",  "mB006.ini",  "mB007.ini", "mB008.ini", "mB009.ini", "mB010.ini", 
	  "mB011.ini",  "mB012.ini",  "mB013.ini",  "mB014.ini",  "mB015.ini",  "mB016.ini",  "mB017.ini",  "mB018.ini", "mB019.ini", "mB020.ini", "mB021.ini",
	  "mB001.ini",  "mB022.ini",  "mB023.ini",  "mB024.ini" , "mB001.ini",  "mB001.ini",  "mB001.ini" , "mB001.ini" ,
	  "Cmb001.ini", "Cmb001.ini", "Cmb001.ini", "Cmb001.ini", "Cmb002.ini", "Cmb002.ini", "Cmb002.ini", "Cmb002.ini",	
	  "Cmb003.ini", "Cmb003.ini", "Cmb003.ini", "Cmb003.ini", "Cmb004.ini", "Cmb004.ini", "Cmb004.ini", "Cmb004.ini",
	  "Cmb005.ini", "Cmb006.ini", "Cmb007.ini", "Cmb008.ini", "Cmb009.ini", "Cmb010.ini", "Cmb011.ini", "Cmb012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	{ "mC001.ini",  "mC001.ini",  "mC002.ini",  "mC003.ini",  "mC004.ini",  "mC005.ini",  "mC006.ini",  "mC007.ini", "mC008.ini", "mC009.ini", "mC010.ini", 
	  "mC011.ini",  "mC012.ini",  "mC013.ini",  "mC014.ini",  "mC015.ini",  "mC016.ini",  "mC017.ini",  "mC018.ini", "mC019.ini", "mC020.ini", "mC021.ini",
	  "mC001.ini",  "mC022.ini",  "mC023.ini",  "mC024.ini",  "mC001.ini",  "mC001.ini",  "mC001.ini",  "mC001.ini",
	  "Cmc001.ini", "Cmc001.ini", "Cmc001.ini", "Cmc001.ini", "Cmc002.ini", "Cmc002.ini", "Cmc002.ini", "Cmc002.ini",
	  "Cmc003.ini", "Cmc003.ini", "Cmc003.ini", "Cmc003.ini", "Cmc004.ini", "Cmc004.ini", "Cmc004.ini", "Cmc004.ini",
	  "Cmc005.ini", "Cmc006.ini", "Cmc007.ini", "Cmc008.ini", "Cmc009.ini", "Cmc010.ini", "Cmc011.ini", "Cmc012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	{ "mD001.ini",  "mD001.ini",  "mD002.ini",  "mD003.ini",  "mD004.ini",  "mD005.ini",  "mD006.ini",  "mD007.ini", "mD008.ini", "mD009.ini", "mD010.ini",
	  "mD011.ini",  "mD012.ini",  "mD013.ini",  "mD014.ini",  "mD015.ini",  "mD016.ini",  "mD017.ini",  "mD018.ini", "mD019.ini", "mD020.ini", "mD021.ini",
	  "mD001.ini",  "mD022.ini",  "mD023.ini",  "mD024.ini",  "mD001.ini",  "mD001.ini",  "mD001.ini",  "mD001.ini",
	  "Cmd001.ini", "Cmd001.ini", "Cmd001.ini", "Cmd001.ini", "Cmd002.ini", "Cmd002.ini", "Cmd002.ini", "Cmd002.ini",
	  "Cmd003.ini", "Cmd003.ini", "Cmd003.ini", "Cmd003.ini", "Cmd004.ini", "Cmd004.ini", "Cmd004.ini", "Cmd004.ini",
	  "Cmd005.ini", "Cmd006.ini", "Cmd007.ini", "Cmd008.ini", "Cmd009.ini", "Cmd010.ini", "Cmd011.ini", "Cmd012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	// Assassin (JOB_CODE 9)
	{
	  "e001.ini", "e001.ini", "e002.ini", "e003.ini", "e004.ini", "e005.ini", "e006.ini", "e007.ini",
	  "e008.ini", "e009.ini", "e010.ini", "e011.ini", "e012.ini", "e013.ini", "e014.ini", "e015.ini",
	  "e016.ini", "e017.ini", "e018.ini", "e019.ini", "e020.ini", "e021.ini", "e001.ini", "e022.ini",
	  "e023.ini", "e024.ini", "e001.ini", "e001.ini", "e001.ini", "e001.ini", "Ce001.ini", "Ce001.ini",
	  "Ce001.ini", "Ce001.ini", "Ce002.ini", "Ce002.ini", "Ce002.ini", "Ce002.ini", "Ce003.ini", "Ce003.ini",
	  "Ce003.ini", "Ce003.ini", "Ce004.ini", "Ce004.ini", "Ce004.ini", "Ce004.ini", "Ce005.ini", "Ce006.ini",
	  "Ce007.ini", "Ce008.ini", "Ce009.ini", "Ce010.ini", "Ce011.ini", "Ce012.ini"
	},

	// Shaman (JOB_CODE 10)
	{
	  "me001.ini", "me001.ini", "me002.ini", "me003.ini", "me004.ini", "me005.ini", "me006.ini", "me007.ini",
	  "me008.ini", "me009.ini", "me010.ini", "me011.ini", "me012.ini", "me013.ini", "me014.ini", "me015.ini",
	  "me016.ini", "me017.ini", "me018.ini", "me019.ini", "me020.ini", "me021.ini", "me001.ini", "me022.ini",
	  "me023.ini", "me024.ini", "me001.ini", "me001.ini", "me001.ini", "me001.ini", "Cme001.ini", "Cme001.ini",
	  "Cme001.ini", "Cme001.ini", "Cme002.ini", "Cme002.ini", "Cme002.ini", "Cme002.ini", "Cme003.ini", "Cme003.ini",
	  "Cme003.ini", "Cme003.ini", "Cme004.ini", "Cme004.ini", "Cme004.ini", "Cme004.ini", "Cme005.ini", "Cme006.ini",
	  "Cme007.ini", "Cme008.ini", "Cme009.ini", "Cme010.ini", "Cme011.ini", "Cme012.ini"
	},

	// Martial Artist (JOB_CODE 11)
	{
	  "f001.ini", "f001.ini", "f002.ini", "f003.ini", "f004.ini", "f005.ini", "f006.ini", "f007.ini",
	  "f008.ini", "f009.ini", "f010.ini", "f011.ini", "f012.ini", "f013.ini", "f014.ini", "f015.ini",
	  "f016.ini", "f017.ini", "f018.ini", "f019.ini", "f020.ini", "f021.ini", "f001.ini", "f022.ini",
	  "f023.ini", "f024.ini", "f001.ini", "f001.ini", "f001.ini", "f001.ini", "Cf001.ini", "Cf001.ini",
	  "Cf001.ini", "Cf001.ini", "Cf002.ini", "Cf002.ini", "Cf002.ini", "Cf002.ini", "Cf003.ini", "Cf003.ini",
	  "Cf003.ini", "Cf003.ini", "Cf004.ini", "Cf004.ini", "Cf004.ini", "Cf004.ini", "Cf005.ini", "Cf006.ini",
	  "f001.ini", "Cf008.ini", "f001.ini", "Cf010.ini", "Cf011.ini", "Cf012.ini"
	}
};
//코믹 옷 모델 목록
char *szArmorNumFile_Comic[ARMOR_JOB_ROWS][ARMOR_FILE_MAX] = {
	{ "A001.ini" , "A001.ini",  "A002.ini",  "A003.ini",  "A004.ini",  "A005.ini",  "A006.ini",  "A007.ini", "A008.ini", "A009.ini", "A010.ini", 
	  "A011.ini" , "A012.ini",  "A013.ini",  "A014.ini",  "A015.ini",  "A016.ini",  "A017.ini",  "A018.ini", "A019.ini", "A020.ini", "A021.ini",  
	  "A001.ini" , "A022.ini",  "A023.ini",  "A024.ini",  "A001.ini",  "A001.ini",  "A001.ini",  "A001.ini",
	  "Ca001.ini", "Ca001.ini", "Ca001.ini", "Ca001.ini", "Ca002.ini", "Ca002.ini", "Ca002.ini", "Ca002.ini",
	  "Ca003.ini", "Ca003.ini", "Ca003.ini", "Ca003.ini", "Ca004.ini", "Ca004.ini", "Ca004.ini", "Ca004.ini",
	  "Ca005.ini", "Ca006.ini", "Ca007.ini", "Ca008.ini", "Ca009.ini", "Ca010.ini", "Ca011.ini", "Ca012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	{ "B001.ini" , "B001.ini",  "B002.ini",  "B003.ini",  "B004.ini" , "B005.ini",  "B006.ini",  "B007.ini", "B008.ini", "B009.ini", "B010.ini" , 
	  "B011.ini" , "B012.ini",  "B013.ini",  "B014.ini",  "B015.ini" , "B016.ini",  "B017.ini",  "B018.ini", "B019.ini", "B020.ini", "B021.ini" ,
	  "B001.ini" , "B022.ini",  "B023.ini",  "B024.ini",  "B001.ini",  "B001.ini" , "B001.ini" , "B001.ini",
	  "Cb001.ini", "Cb001.ini", "Cb001.ini", "Cb001.ini", "Cb002.ini", "Cb002.ini", "Cb002.ini", "Cb002.ini",	
	  "Cb003.ini", "Cb003.ini", "Cb003.ini", "Cb003.ini", "Cb004.ini", "Cb004.ini", "Cb004.ini", "Cb004.ini",
	  "Cb005.ini", "Cb006.ini", "Cb007.ini", "Cb008.ini", "Cb009.ini", "Cb010.ini", "Cb011.ini", "Cb012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	
	{ "C001.ini" , "C001.ini",  "C002.ini",  "C003.ini",  "C004.ini",  "C005.ini",  "C006.ini",  "C007.ini",  "C008.ini", "C009.ini", "C010.ini",
	  "C011.ini" , "C012.ini",  "C013.ini",  "C014.ini",  "C015.ini",  "C016.ini",  "C017.ini",  "C018.ini",  "C019.ini", "C020.ini", "C021.ini",
	  "C001.ini" , "C022.ini",  "C023.ini",  "C024.ini",  "C001.ini",  "C001.ini",  "C001.ini",  "C001.ini",
	  "Cc001.ini", "Cc001.ini", "Cc001.ini", "Cc001.ini", "Cc002.ini", "Cc002.ini", "Cc002.ini", "Cc002.ini",
	  "Cc003.ini", "Cc003.ini", "Cc003.ini", "Cc003.ini", "Cc004.ini", "Cc004.ini", "Cc004.ini", "Cc004.ini",
	  "Cc005.ini", "Cc006.ini", "Cc007.ini", "Cc008.ini", "Cc009.ini", "Cc010.ini", "Cc011.ini", "Cc012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	{ "D001.ini" , "D001.ini",  "D002.ini",  "D003.ini",  "D004.ini",  "D005.ini",  "D006.ini",  "D007.ini", "D008.ini", "D009.ini", "D010.ini", 
	  "D011.ini" , "D012.ini",  "D013.ini",  "D014.ini",  "D015.ini",  "D016.ini",  "D017.ini",  "D018.ini", "D019.ini", "D020.ini", "D021.ini",
	  "D001.ini" , "D022.ini",  "D023.ini",  "D024.ini",  "D001.ini",  "D001.ini",  "D001.ini",  "D001.ini",
	  "Cd001.ini", "Cd001.ini", "Cd001.ini", "Cd001.ini", "Cd002.ini", "Cd002.ini", "Cd002.ini", "Cd002.ini",
	  "Cd003.ini", "Cd003.ini", "Cd003.ini", "Cd003.ini", "Cd004.ini", "Cd004.ini", "Cd004.ini", "Cd004.ini",
	  "Cd005.ini", "Cd006.ini", "Cd007.ini", "Cd008.ini", "Cd009.ini", "Cd010.ini", "Cd011.ini", "Cd012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	{ "mA001.ini",  "mA001.ini",  "mA002.ini",  "mA003.ini",   "mA004.ini",  "mA005.ini",  "mA006.ini",  "mA007.ini", "mA008.ini", "mA009.ini", "mA010.ini", 
	  "mA011.ini",  "mA012.ini",  "mA013.ini",  "mA014.ini",   "mA015.ini",  "mA016.ini",  "mA017.ini",  "mA018.ini", "mA019.ini", "mA020.ini", "mA021.ini",
	  "mA001.ini",  "mA022.ini",  "mA023.ini",  "mA024.ini",  "mA001.ini",   "mA001.ini",  "mA001.ini",  "mA001.ini",
	  "Cma001.ini", "Cma001.ini", "Cma001.ini", "Cma001.ini",  "Cma002.ini", "Cma002.ini", "Cma002.ini", "Cma002.ini",
	  "Cma003.ini", "Cma003.ini", "Cma003.ini", "Cma003.ini",  "Cma004.ini", "Cma004.ini", "Cma004.ini", "Cma004.ini",
	  "Cma005.ini", "Cma006.ini", "Cma007.ini", "Cma008.ini",  "Cma009.ini",  "Cma010.ini", "Cma011.ini", "Cma012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},
	
	{ "mB001.ini",  "mB001.ini",  "mB002.ini",  "mB003.ini",  "mB004.ini",  "mB005.ini",  "mB006.ini",  "mB007.ini", "mB008.ini", "mB009.ini", "mB010.ini", 
	  "mB011.ini",  "mB012.ini",  "mB013.ini",  "mB014.ini",  "mB015.ini",  "mB016.ini",  "mB017.ini",  "mB018.ini", "mB019.ini", "mB020.ini", "mB021.ini",
	  "mB001.ini",  "mB022.ini",  "mB023.ini",  "mB024.ini" , "mB001.ini",  "mB001.ini",  "mB001.ini" , "mB001.ini" ,
	  "Cmb001.ini", "Cmb001.ini", "Cmb001.ini", "Cmb001.ini", "Cmb002.ini", "Cmb002.ini", "Cmb002.ini", "Cmb002.ini",	
	  "Cmb003.ini", "Cmb003.ini", "Cmb003.ini", "Cmb003.ini", "Cmb004.ini", "Cmb004.ini", "Cmb004.ini", "Cmb004.ini",
	  "Cmb005.ini", "Cmb006.ini", "Cmb007.ini", "Cmb008.ini", "Cmb009.ini", "Cmb010.ini", "Cmb011.ini", "Cmb012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	{ "mC001.ini",  "mC001.ini",  "mC002.ini",  "mC003.ini",  "mC004.ini",  "mC005.ini",  "mC006.ini",  "mC007.ini", "mC008.ini", "mC009.ini", "mC010.ini", 
	  "mC011.ini",  "mC012.ini",  "mC013.ini",  "mC014.ini",  "mC015.ini",  "mC016.ini",  "mC017.ini",  "mC018.ini", "mC019.ini", "mC020.ini", "mC021.ini",
	  "mC001.ini",  "mC022.ini",  "mC023.ini",  "mC024.ini",  "mC001.ini",  "mC001.ini",  "mC001.ini",  "mC001.ini",
	  "Cmc001.ini", "Cmc001.ini", "Cmc001.ini", "Cmc001.ini", "Cmc002.ini", "Cmc002.ini", "Cmc002.ini", "Cmc002.ini",
	  "Cmc003.ini", "Cmc003.ini", "Cmc003.ini", "Cmc003.ini", "Cmc004.ini", "Cmc004.ini", "Cmc004.ini", "Cmc004.ini",
	  "Cmc005.ini", "Cmc006.ini", "Cmc007.ini", "Cmc008.ini", "Cmc009.ini", "Cmc010.ini", "Cmc011.ini", "Cmc012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	{ "mD001.ini",  "mD001.ini",  "mD002.ini",  "mD003.ini",  "mD004.ini",  "mD005.ini",  "mD006.ini",  "mD007.ini", "mD008.ini", "mD009.ini", "mD010.ini",
	  "mD011.ini",  "mD012.ini",  "mD013.ini",  "mD014.ini",  "mD015.ini",  "mD016.ini",  "mD017.ini",  "mD018.ini", "mD019.ini", "mD020.ini", "mD021.ini",
	  "mD001.ini",  "mD022.ini",  "mD023.ini",  "mD024.ini",  "mD001.ini",  "mD001.ini",  "mD001.ini",  "mD001.ini",
	  "Cmd001.ini", "Cmd001.ini", "Cmd001.ini", "Cmd001.ini", "Cmd002.ini", "Cmd002.ini", "Cmd002.ini", "Cmd002.ini",
	  "Cmd003.ini", "Cmd003.ini", "Cmd003.ini", "Cmd003.ini", "Cmd004.ini", "Cmd004.ini", "Cmd004.ini", "Cmd004.ini",
	  "Cmd005.ini", "Cmd006.ini", "Cmd007.ini", "Cmd008.ini", "Cmd009.ini", "Cmd010.ini", "Cmd011.ini", "Cmd012.ini" // 박재원 - 2010 월드컵 이벤트(축구복 추가), 수영복 추가
	},

	// Assassin (JOB_CODE 9)
	{
	  "e001.ini", "e001.ini", "e002.ini", "e003.ini", "e004.ini", "e005.ini", "e006.ini", "e007.ini",
	  "e008.ini", "e009.ini", "e010.ini", "e011.ini", "e012.ini", "e013.ini", "e014.ini", "e015.ini",
	  "e016.ini", "e017.ini", "e018.ini", "e019.ini", "e020.ini", "e021.ini", "e001.ini", "e022.ini",
	  "e023.ini", "e024.ini", "e001.ini", "e001.ini", "e001.ini", "e001.ini", "Ce001.ini", "Ce001.ini",
	  "Ce001.ini", "Ce001.ini", "Ce002.ini", "Ce002.ini", "Ce002.ini", "Ce002.ini", "Ce003.ini", "Ce003.ini",
	  "Ce003.ini", "Ce003.ini", "Ce004.ini", "Ce004.ini", "Ce004.ini", "Ce004.ini", "Ce005.ini", "Ce006.ini",
	  "Ce007.ini", "Ce008.ini", "Ce009.ini", "Ce010.ini", "Ce011.ini", "Ce012.ini"
	},

	// Shaman (JOB_CODE 10)
	{
	  "me001.ini", "me001.ini", "me002.ini", "me003.ini", "me004.ini", "me005.ini", "me006.ini", "me007.ini",
	  "me008.ini", "me009.ini", "me010.ini", "me011.ini", "me012.ini", "me013.ini", "me014.ini", "me015.ini",
	  "me016.ini", "me017.ini", "me018.ini", "me019.ini", "me020.ini", "me021.ini", "me001.ini", "me022.ini",
	  "me023.ini", "me024.ini", "me001.ini", "me001.ini", "me001.ini", "me001.ini", "Cme001.ini", "Cme001.ini",
	  "Cme001.ini", "Cme001.ini", "Cme002.ini", "Cme002.ini", "Cme002.ini", "Cme002.ini", "Cme003.ini", "Cme003.ini",
	  "Cme003.ini", "Cme003.ini", "Cme004.ini", "Cme004.ini", "Cme004.ini", "Cme004.ini", "Cme005.ini", "Cme006.ini",
	  "Cme007.ini", "Cme008.ini", "Cme009.ini", "Cme010.ini", "Cme011.ini", "Cme012.ini"
	},

	// Martial Artist (JOB_CODE 11)
	{
	  "f001.ini", "f001.ini", "f002.ini", "f003.ini", "f004.ini", "f005.ini", "f006.ini", "f007.ini",
	  "f008.ini", "f009.ini", "f010.ini", "f011.ini", "f012.ini", "f013.ini", "f014.ini", "f015.ini",
	  "f016.ini", "f017.ini", "f018.ini", "f019.ini", "f020.ini", "f021.ini", "f001.ini", "f022.ini",
	  "f023.ini", "f024.ini", "f001.ini", "f001.ini", "f001.ini", "f001.ini", "Cf001.ini", "Cf001.ini",
	  "Cf001.ini", "Cf001.ini", "Cf002.ini", "Cf002.ini", "Cf002.ini", "Cf002.ini", "Cf003.ini", "Cf003.ini",
	  "Cf003.ini", "Cf003.ini", "Cf004.ini", "Cf004.ini", "Cf004.ini", "Cf004.ini", "Cf005.ini", "Cf006.ini",
	  "f001.ini", "Cf008.ini", "f001.ini", "Cf010.ini", "Cf011.ini", "Cf012.ini"
	}
};

/*
//일반 옷 모델 목록
char *szArmorNumFile[8][18] = {
	{ "A001.ini", "A001.ini", "A004.ini", "A004.ini", "A004.ini", "A005.ini", "A005.ini", "A005.ini" , "A008.ini" , "A008.ini" , "A008.ini" , "A010.ini" , "A010.ini" , "A010.ini" , "A011.ini" , "A011.ini" , "A011.ini" , "A013.ini" },
	{ "B001.ini", "B001.ini", "B003.ini", "B003.ini", "B003.ini", "B006.ini", "B006.ini", "B006.ini" , "B008.ini" , "B008.ini" , "B008.ini" , "B010.ini" , "B010.ini" , "B010.ini" , "B011.ini" , "B011.ini" , "B011.ini" , "B013.ini" },
	{ "C001.ini", "C001.ini", "C003.ini", "C003.ini", "C003.ini", "C006.ini", "C006.ini", "C006.ini" , "C008.ini" , "C008.ini" , "C008.ini" , "C010.ini" , "C010.ini" , "C010.ini" , "C011.ini" , "C011.ini" , "C011.ini" , "C013.ini" },
	{ "D001.ini", "D001.ini", "D003.ini", "D003.ini", "D003.ini", "D006.ini", "D006.ini", "D006.ini" , "D008.ini" , "D008.ini" , "D008.ini" , "D010.ini" , "D010.ini" , "D010.ini" , "D012.ini" , "D012.ini" , "D012.ini" , "D013.ini" },

	{ "mA001.ini", "mA001.ini", "mA004.ini", "mA004.ini", "mA004.ini", "mA006.ini", "mA006.ini", "mA006.ini" , "mA007.ini" , "mA007.ini" , "mA007.ini" , "mA011.ini" , "mA011.ini" , "mA011.ini" , "mA015.ini" , "mA015.ini" , "mA015.ini" , "mA017.ini" },
	{ "mB001.ini", "mB001.ini", "mB003.ini", "mB003.ini", "mB003.ini", "mB005.ini", "mB005.ini", "mB005.ini" , "mB009.ini" , "mB009.ini" , "mB009.ini" , "mB011.ini" , "mB011.ini" , "mB011.ini" , "mB015.ini" , "mB015.ini" , "mB015.ini" , "mB017.ini" },
	{ "mC001.ini", "mC001.ini", "mC003.ini", "mC003.ini", "mC003.ini", "mC005.ini", "mC005.ini", "mC005.ini" , "mC007.ini" , "mC007.ini" , "mC007.ini" , "mC008.ini" , "mC008.ini" , "mC008.ini" , "mC009.ini" , "mC009.ini" , "mC009.ini" , "mC010.ini" },
	{ "mD001.ini", "mD001.ini", "mD003.ini", "mD003.ini", "mD003.ini", "mD005.ini", "mD005.ini", "mD005.ini" , "mD007.ini" , "mD007.ini" , "mD007.ini" , "mD008.ini" , "mD008.ini" , "mD008.ini" , "mD009.ini" , "mD009.ini" , "mD009.ini" , "mD010.ini" }
};
//코믹 옷 모델 목록
char *szArmorNumFile_Comic[8][18] = {

	{ "A001t.ini", "A001t.ini", "A004t.ini", "A004t.ini", "A004t.ini", "A005t.ini", "A005t.ini", "A005t.ini" , "A008t.ini" , "A008t.ini" , "A008t.ini" , "A010t.ini" , "A010t.ini" , "A010t.ini" , "A011t.ini" , "A011t.ini" , "A011t.ini" , "A013t.ini" },
	{ "B001t.ini", "B001t.ini", "B003t.ini", "B003t.ini", "B003t.ini", "B006t.ini", "B006t.ini", "B006t.ini" , "B008t.ini" , "B008t.ini" , "B008t.ini" , "B010t.ini" , "B010t.ini" , "B010t.ini" , "B011t.ini" , "B011t.ini" , "B011t.ini" , "B013t.ini" },
	{ "C001t.ini", "C001t.ini", "C003t.ini", "C003t.ini", "C003t.ini", "C006t.ini", "C006t.ini", "C006t.ini" , "C008t.ini" , "C008t.ini" , "C008t.ini" , "C010t.ini" , "C010t.ini" , "C010t.ini" , "C011t.ini" , "C011t.ini" , "C011t.ini" , "C013t.ini" },
	{ "D001t.ini", "D001t.ini", "D003t.ini", "D003t.ini", "D003t.ini", "D006t.ini", "D006t.ini", "D006t.ini" , "D008t.ini" , "D008t.ini" , "D008t.ini" , "D010t.ini" , "D010t.ini" , "D010t.ini" , "D012t.ini" , "D012t.ini" , "D012t.ini" , "D013t.ini" },

	{ "mA001t.ini", "mA001t.ini", "mA004t.ini", "mA004t.ini", "mA004t.ini", "mA006t.ini", "mA006t.ini", "mA006t.ini" , "mA007t.ini" , "mA007t.ini" , "mA007t.ini" , "mA011t.ini" , "mA011t.ini" , "mA011t.ini" , "mA015t.ini" , "mA015t.ini" , "mA015t.ini" , "mA017t.ini" },
	{ "mB001t.ini", "mB001t.ini", "mB003t.ini", "mB003t.ini", "mB003t.ini", "mB005t.ini", "mB005t.ini", "mB005t.ini" , "mB009t.ini" , "mB009t.ini" , "mB009t.ini" , "mB011t.ini" , "mB011t.ini" , "mB011t.ini" , "mB015t.ini" , "mB015t.ini" , "mB015t.ini" , "mB017t.ini" },
	{ "mC001t.ini", "mC001t.ini", "mC003t.ini", "mC003t.ini", "mC003t.ini", "mC005t.ini", "mC005t.ini", "mC005t.ini" , "mC007t.ini" , "mC007t.ini" , "mC007t.ini" , "mC008t.ini" , "mC008t.ini" , "mC008t.ini" , "mC009t.ini" , "mC009t.ini" , "mC009t.ini" , "mC010t.ini" },
	{ "mD001t.ini", "mD001t.ini", "mD003t.ini", "mD003t.ini", "mD003t.ini", "mD005t.ini", "mD005t.ini", "mD005t.ini" , "mD007t.ini" , "mD007t.ini" , "mD007t.ini" , "mD008t.ini" , "mD008t.ini" , "mD008t.ini" , "mD009t.ini" , "mD009t.ini" , "mD009t.ini" , "mD010t.ini" }
};
*/



