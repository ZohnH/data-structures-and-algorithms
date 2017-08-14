#pragma  once 

#include "Dictionary.h"
#include "Bitmap.h"
#include "prime.h"
#include "../BTree/release.h"


template <class K, class V> //key��value
class Hashtable : public Dictionary<K, V>
{
private:
	Entry<K, V> **ht; //Ͱ���飬��Ŵ���ָ��
	int M; //Ͱ��������
	int N; //��������
	Bitmap *lazyRemoval; //����ɾ�����
#define lazilyRemoved(x) (lazyRemoval->test(x))
#define markAsRemoved(x)  (lazyRemoval->set(x))

protected:
	int probe4Hit(const K& k); //�عؼ���k��Ӧ�Ĳ��������ҵ�����ƥ���Ͱ
	int probe4Free(const K& k); //�عؼ���k��Ӧ�Ĳ��������ҵ��׸����ÿ�Ͱ
	void rehash(); //��ɢ���㷨������Ͱ���飬��֤װ�������ھ���������

public:
	Hashtable(int c = 5); //����һ��������С��c��ɢ�б���Ϊ������ʱѡ�ý�С��Ĭ��ֵ��
	~Hashtable(); //�ͷ�Ͱ���鼰���и����ǿգ�Ԫ����ָ��Ĵ���
	int size() const { return N; } // ��ǰ�Ĵ�����Ŀ
	bool put(K, V); //���루��ֹ��ͬ�������ʿ���ʧ�ܣ�
	V* get(K k); //��ȡ
	bool remove(K k); //ɾ��
};
;
;


template<class K, class V>
Hashtable<K, V>::Hashtable(int c)  //����ɢ�б�������Ϊ
{
	M = primeNLT(c, 1048576, "prime-1048576-bitmap.txt"); //��С��c������M
	N = 0;
	ht = new Entry<K, V> *[M]; //����Ͱ���飨����˶�����ɹ�������ʼװ������ΪN/M = 0%
	memset(ht, 0, sizeof(Entry<K, V>*) * M); //��ʼ����Ͱ
	lazyRemoval = new Bitmap(M); //����ɾ����Ǳ���ͼ
}

template <class K, class V>
Hashtable<K, V>::~Hashtable() //����ǰ�ͷ�Ͱ���鼰�ǿմ���
{
	for (int i = 0; i < M; ++i) //��һ����Ͱ
	{
		if (ht[i])
			release(ht[i]); //�ͷŷǿյ�Ͱ
	}
	release(ht); //�ͷ�Ͱ����
	release(lazyRemoval); //�ͷ�����ɾ�����
}

static size_t hashCode(char c){ return (size_t)c; }//�ַ�
static size_t hashCode(int k){ return (size_t)k; } //�����Լ���������
static size_t hashCode(long long i){ return (size_t)((i >> 32) + (int)i); }
static size_t hashCode(char s[])//�����ַ�����ѭ����λɢ���루cyclic shift hash code��
{
	int h = 0; //ɢ����
	for (size_t n = strlen(s), i = 0; i < n; i++)//�������ң��������ÿһ�ַ�
	{
		h = (h << 5) | (h >> 27); 
		h += (int)s[i];          //ɢ����ѭ������5λ�����ۼӵ�ǰ�ַ�
	}
	return (size_t)h;  //������õ�ɢ���룬ʵ���Ͽ�����Ϊ���Ƶġ�����ʽɢ���롱
} //����Ӣ�ﵥ�ʣ�"ѭ������5λ"��ʵ��ͳ�Ƶó������ֵ


/******************************************************************
* �عؼ���k��Ӧ�Ĳ��������ҵ���֮ƥ���Ͱ�������Һ�ɾ������ʱ���ã�
* ��̽���Զ��ֶ����������ѡȡ���������������̽����Ϊ��
******************************************************************/
template <class K, class V>
int Hashtable<K, V>::probe4Hit(const K& k)
{
	int r = hashCode(k) % M;  //����ʼͰ�������෨ȷ��������
	while ( (ht[r] && (k != ht[r]->key)) || (!ht[r] && lazilyRemoved(r)) )
		r = (r + 1) % M;  //�ز�����������̽���������г�ͻ��Ͱ���Լ�������ɾ����ǵ�Ͱ
	return r; //�����߸���ht[r]�Ƿ�Ϊ�գ������жϲ����Ƿ�ɹ�
}

/**********************************************************
* �عؼ���k��Ӧ�Ĳ��������ҵ��׸����ÿ�Ͱ�������������ʱ���ã�
* ��̽���Զ��ֶ����������ѡȡ���������������̽����Ϊ��
**********************************************************/
template <class K, class V>
int Hashtable<K, V>::probe4Free(const K& k)
{
	int r = hashCode(k) % M;  //����ʼͰ�������෨ȷ��������
	while (ht[r]) 
		r = (r + 1) % M;  //�ز�������Ͱ��̽��ֱ���׸���Ͱ�������Ƿ��������ɾ����ǣ�
	return r; //Ϊ��֤��Ͱ�����ҵ���װ�����Ӽ�ɢ�б�����Ҫ��������
}

template<class K, class V>
V* Hashtable<K, V>::get(K k)//ɢ�б����������㷨
{
	int r = probe4Hit(k);
	return ht[r] ? &(ht[r]->value) : NULL;
}//��ֹ������keyֵ��ͬ

template<class K, class V>
bool Hashtable<K, V>::put(K k, V v) //ɢ�б���������
{
	if (ht[probe4Hit(k)])
		return false;  //��ͬԪ�ز����ظ�����
	int r = probe4Free(k); //Ϊ�´����Ҹ���Ͱ��ֻҪװ�����ӿ��Ƶõ�����Ȼ�ɹ���
	ht[r] = new Entry<K, V>(k, v);  ++N; //���루ע�⣺����ɾ��������踴λ��
	if (N * 2 > M)
		rehash();  //װ�����Ӹ���50%����ɢ��
	return true;
}

template<class K, class V>
bool Hashtable<K, V>::remove(K k)//ɢ�б�����ɾ���㷨
{
	int r = probe4Hit(k);
	if (!ht[r])
		return false;   //��Ӧ����������ʱ���޷�ɾ��
	release(ht[r]); //�����ͷ�Ͱ�д�������������ɾ����ǣ������´�������
	ht[r] = nullptr;
	markAsRemoved(r);
	N--;
	return true;
}

/******************************************************************************************
* ��ɢ���㷨��װ�����ӹ���ʱ����ȡ����һȡ���ٲ��롱�����ز��ԣ���Ͱ��������
* ���ɼ򵥵أ�ͨ��memcpy()����ԭͰ���鸴�Ƶ���Ͱ���飨����ǰ�ˣ�����������������⣺
* 1����̳�ԭ�г�ͻ��2�����ܵ��²������ں�˶��ѡ�������Ϊ��������Ͱ��������ɾ����־Ҳ�޼�����
******************************************************************************************/
template<class K, class V>
void Hashtable<K, V>::rehash()
{
	int old_capacity = M;
	Entry<K, V>** old_ht = ht;
	M = primeNLT(2 * M, 1048576, "prime-1048576-bitmap.txt");//�������ټӱ�
	N = 0; ht = new Entry<K, V> *[M];
	memset(ht, 0, sizeof(Entry<K, V>*)*M);//�¿�����ɾ����Ǳ���ͼ
	for (int i = 0; i < old_capacity; ++i)//ɨ��ԭͰ����
	{
		if (old_ht[i]) //���ǿ�Ͱ�еĴ�����һ
			put(old_ht[i]->key, old_ht[i]->value);  //�������µ�Ͱ����
	}
	release(old_ht);//�ͷ�ԭͰ���顪����������ԭ�ȴ�ŵĴ�������ת�ƣ���ֻ���ͷ�Ͱ���鱾��
}