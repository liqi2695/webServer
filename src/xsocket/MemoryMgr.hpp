#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include<stdlib.h>
#include<assert.h>
class MemoryAlloc;

#ifdef _DEBUG
#include<stdio.h>
	#define xPrintf(...) printf(__VA_ARGS__)
#else
	#define xPrintf(...)

#endif

#define MAX_MEMORY_SIZE  1024
//�ڴ�� ��С��Ԫ
class MemoryBlock
{
public:
	//�ڴ����
	int nID;
	//���ô���
	int nRef;
	//��������ڴ��
	MemoryAlloc* pAlloc;
	//��һ��λ��
	MemoryBlock* pNext;
	//�Ƿ����ڴ����
	bool bPool;
private:
	//Ԥ��
	char c1;
	char c2;
	char c3;

};
//int MemoryBlockSize = sizeof(MemoryBlock);
//4+4+4+4+4 x86 ���ֽڶ���   4+4+8+8+8 x64���ֽڶ���

//�ڴ��
class MemoryAlloc
{
public:
	MemoryAlloc()
	{
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlockSize = 0;

	}
	~MemoryAlloc()
	{
		if (_pBuf)
			free(_pBuf);
	}
	///�����ڴ�
	void* allocMemory(size_t nSize)
	{
		if (!_pBuf)
		{
			initMemory();
		}
		MemoryBlock* pReturn = nullptr;//����������ڴ�ͷ�ڵ�
		//�ж���û�е�Ԫ��
		if (nullptr == _pHeader)
		{
			//�û�������ڴ�+�ڴ��Ŀռ�
			pReturn =reinterpret_cast<MemoryBlock*>(malloc(nSize+sizeof(MemoryBlock)));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		else
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocMem:%11x,id = %d, size = %d \n", pReturn, pReturn->nID, nSize);
		return (reinterpret_cast<char*>(pReturn)+sizeof(MemoryBlock));
	}
	///�ͷ��ڴ�
	void freeMemory(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);//����,nRefֵ�϶�Ϊ1
		//û�б����ù�
		if (--pBlock->nRef != 0)
		{
			return;
		}
		//�ڳ��ڡ�������һ��ָ��ǰͷ������ǰͷ��һ�����ã�,Ҳ���Ǳ��ͷ
		if (pBlock->bPool)
		{
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		//�ڳ���ֱ��free
		else
		{
			free(pBlock);
		}

	}

	//��ʼ���ڴ��
	void initMemory()
	{
		//����
		assert(nullptr==_pBuf);
		if (_pBuf)
			return;
		//��ϵͳ����ص��ڴ�
		size_t bufsize = (_nSize + sizeof(MemoryBlock)) * _nBlockSize; //�����С��(��Ԫ�����+MemoryBlock)*��Ԫ���С
		_pBuf = static_cast<char*>(malloc(bufsize));
		//��ʼ���ڴ��
		_pHeader = reinterpret_cast<MemoryBlock*>(_pBuf);
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;

		//�����ڴ����г�ʼ��
		MemoryBlock* pTemp1 = _pHeader;
		for (size_t n = 1; n < _nBlockSize; n++)
		{
			MemoryBlock*  pTemp2 = reinterpret_cast<MemoryBlock*>(_pBuf+n* (_nSize+sizeof(MemoryBlock)));
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr; //��֤�������һ����next��Ϊ��
			pTemp1->pNext = pTemp2;  //����
			pTemp1 = pTemp2;  //ǰ��
		}
	}
protected:
	//�ڴ�ص�ַ
	char* _pBuf;
	//ͷ���ڴ浥Ԫ
	MemoryBlock* _pHeader;
	//�ڴ浥Ԫ�Ĵ�С
	size_t _nSize;
	//�ڴ浥Ԫ������
	size_t _nBlockSize;

};

//ģ��,ʹ��ģ����г�ʼ��
template<size_t nSize ,size_t nBlockSize>
class MemoryAlloctor:public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		const size_t n = sizeof(void*);
		//������nSizeΪ61  ����������Ϊ 61/8 = 7*8 +��61/8��8��0�� = 64 
		_nSize = (nSize/n)*n + (nSize % n ? n : 0);//��֤�ڴ����,�������61���Զ�����Ϊ64
		_nBlockSize = nBlockSize;
	}
};

//�ڳ�Ա��������ʱȡ��ʼ��ֵ
//�ڴ��������
class MemoryMgr
{
private:
	MemoryMgr()
	{
		init_szAlloc(0, 64, &_mem64);
		init_szAlloc(65, 128, &_mem128);
		init_szAlloc(129,256, &_mem256);
		init_szAlloc(257, 512, &_mem512);
		init_szAlloc(513, 1024, &_mem1024);
	}
	~MemoryMgr()
	{

	}

public:
	//����ģʽ,��֤��������������ҽ���һ������
	//һ����ľ�̬������û�ж����������ǿ���ֱ�ӷ��ʵ�
	//ʵ���ڴ�أ�����ֻ��Ҫȫ�ִ���һ�����󼴿ɣ�����Ҫ�������
	//��̬�����ǲ��ᱻ�ͷŵ�
	static MemoryMgr& Instance()
	{
		static MemoryMgr mgr;
		return mgr;
	}

	//�����ڴ�
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SIZE) //�ڳ��У�������
		{
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else//���ڣ��Լ�����ȥ����
		{
			MemoryBlock* pReturn = reinterpret_cast<MemoryBlock*>(malloc(nSize + sizeof(MemoryBlock)));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			xPrintf("allocMem:%11x,id = %d, size = %d \n",pReturn,pReturn->nID,nSize );
			return (reinterpret_cast<char*>(pReturn) + sizeof(MemoryBlock));
		}
		
	}
	///�ͷ��ڴ�
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		xPrintf("freeMem:%11x,id = %d \n", pBlock,pBlock->nID);
		if (pBlock->bPool)//�ڳ��У������ع���
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else//���ڳ���
		{
			if (--pBlock->nRef == 0)
				free(pBlock);
		}
	}
	//�����ڴ������ü���
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//��ʼ���ڴ��ӳ������
	//��nBegin��nEnd����ֽڷ�Χ��ӳ�����飬ָ���ض����ڴ��
	void init_szAlloc(int nBegin, int nEnd , MemoryAlloc* pMemA)
	{
		for (int i = nBegin; i <= nEnd; i++)
		{
			_szAlloc[i] = pMemA;
		}
	}
private:
	MemoryAlloctor<64, 100000> _mem64;
	//��ʱ�Ͳ���Ҫȥ����������д��β����ˣ��൱����������Ϳ���ֱ�Ӹ�ֵ
	MemoryAlloctor<128, 100000> _mem128;
	MemoryAlloctor<256, 100000> _mem256;
	MemoryAlloctor<512, 100000> _mem512;
	MemoryAlloctor<1024, 100000> _mem1024;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
	//����64+1�񣬶�Ӧ�����������64����Ϊc++����������0��ʼ������Ҫ+1
};


#endif // !_MemoryMgr_hpp_

