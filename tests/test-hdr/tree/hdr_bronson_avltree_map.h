//$$CDS-header$$

#ifndef CDSTEST_HDR_BRONSON_AVLTREE_MAP_H
#define CDSTEST_HDR_BRONSON_AVLTREE_MAP_H

#include "cppunit/cppunit_proxy.h"
#include "size_check.h"
#include <functional>   // ref
#include <algorithm>

namespace tree {
    using misc::check_size;

    class BronsonAVLTreeHdrTest : public CppUnitMini::TestCase
    {
    public:
        typedef int     key_type;

        struct stat_data {
            size_t  nInsertFuncCall;
            size_t  nEnsureExistFuncCall;
            size_t  nEnsureNewFuncCall;
            size_t  nEraseFuncCall;
            size_t  nFindFuncCall;

            stat_data()
                : nInsertFuncCall( 0 )
                , nEnsureExistFuncCall( 0 )
                , nEnsureNewFuncCall( 0 )
                , nEraseFuncCall( 0 )
                , nFindFuncCall( 0 )
            {}
        };

        struct value_type {
            int         nVal;
            stat_data   stat;

            value_type()
            {}

            value_type( int v )
                : nVal( v )
            {}
        };

        struct wrapped_int {
            int  nKey;

            wrapped_int( int n )
                : nKey( n )
            {}
        };

        struct wrapped_less
        {
            bool operator()( wrapped_int const& w, int n ) const
            {
                return w.nKey < n;
            }
            bool operator()( int n, wrapped_int const& w ) const
            {
                return n < w.nKey;
            }
            template <typename T>
            bool operator()( wrapped_int const& w, T const& v ) const
            {
                return w.nKey < v.nKey;
            }
            template <typename T>
            bool operator()( T const& v, wrapped_int const& w ) const
            {
                return v.nKey < w.nKey;
            }
        };

    protected:
        static const size_t c_nItemCount = 10000;

        class data_array
        {
            int *     pFirst;
            int *     pLast;

        public:
            data_array()
                : pFirst( new int[c_nItemCount] )
                , pLast( pFirst + c_nItemCount )
            {
                int i = 0;
                for ( int * p = pFirst; p != pLast; ++p, ++i )
                    *p = i;

                std::random_shuffle( pFirst, pLast );
            }

            ~data_array()
            {
                delete[] pFirst;
            }

            int operator[]( size_t i ) const
            {
                assert( i < size_t( pLast - pFirst ) );
                return pFirst[i];
            }
        };

        struct find_functor
        {
            void operator()( key_type, value_type& v ) const
            {
                ++v.stat.nFindFuncCall;
            }
        };

        template <typename Item>
        struct copy_found
        {
            Item    m_found;

            void operator()( key_type const&, Item& v )
            {
                m_found = v;
            }

            void operator()( Item& v )
            {
                m_found = v;
            }
        };

        struct insert_functor
        {
            template <typename Item>
            void operator()( key_type key, Item& i )
            {
                i.nVal = key * 100;
                ++i.stat.nInsertFuncCall;
            }
        };

        template <typename Q>
        static void ensure_func( bool bNew, Q /*key*/, value_type& i )
        {
            if ( bNew )
                ++i.stat.nEnsureNewFuncCall;
            else
                ++i.stat.nEnsureExistFuncCall;
        }

        struct ensure_functor
        {
            template <typename Q>
            void operator()( bool bNew, Q key, value_type& i )
            {
                ensure_func( bNew, key, i );
            }
        };

        struct extract_functor
        {
            int nKey;

            void operator()( value_type& v )
            {
                nKey = v.nVal;
            }
        };

    protected:
        template <class Set>
        void test_with( Set& s )
        {
            value_type itm;
            int key;

            // insert/find test
            CPPUNIT_ASSERT( !s.find( 10 ) );
            CPPUNIT_ASSERT( s.insert( 10 ) );
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 1 ) );
            CPPUNIT_ASSERT( s.find( 10 ) );

            CPPUNIT_ASSERT( !s.insert( 10 ) );
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 1 ) );

            CPPUNIT_ASSERT( !s.find_with( 20, std::less<key_type>() ) );
            CPPUNIT_ASSERT( s.insert( 20, 25 ) );
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 2 ) );
            CPPUNIT_ASSERT( s.find_with( 10, std::less<key_type>() ) );
            CPPUNIT_ASSERT( s.find( key = 20 ) );
            CPPUNIT_ASSERT( s.find_with( key, std::less<key_type>(), find_functor() ) );
            {
                copy_found<value_type> f;
                key = 20;
                CPPUNIT_ASSERT( s.find( key, std::ref( f ) ) );
                CPPUNIT_ASSERT( f.m_found.nVal == 25 );
                CPPUNIT_ASSERT( f.m_found.stat.nFindFuncCall == 1 );
            }
            CPPUNIT_ASSERT( s.find( key, find_functor() ) );
            {
                copy_found<value_type> f;
                key = 20;
                CPPUNIT_ASSERT( s.find_with( key, std::less<key_type>(), std::ref( f ) ) );
                CPPUNIT_ASSERT( f.m_found.nVal == 25 );
                CPPUNIT_ASSERT( f.m_found.stat.nFindFuncCall == 2 );
            }
            CPPUNIT_ASSERT( s.find( 20, find_functor() ) );
            {
                copy_found<value_type> f;
                CPPUNIT_ASSERT( s.find_with( 20, std::less<key_type>(), std::ref( f ) ) );
                CPPUNIT_ASSERT( f.m_found.nVal == 25 );
                CPPUNIT_ASSERT( f.m_found.stat.nFindFuncCall == 3 );
            }

            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 2 ) );

            CPPUNIT_ASSERT( !s.find( 25 ) );
            CPPUNIT_ASSERT( s.insert_with( 25, insert_functor() ) );
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 3 ) );
            {
                copy_found<value_type> f;
                key = 25;
                CPPUNIT_ASSERT( s.find( key, std::ref( f ) ) );
                CPPUNIT_ASSERT( f.m_found.nVal == 2500 );
                CPPUNIT_ASSERT( f.m_found.stat.nInsertFuncCall == 1 );
            }

            // update test
            key = 10;
            {
                copy_found<value_type> f;
                CPPUNIT_ASSERT( s.find( key, std::ref( f ) ) );
                CPPUNIT_ASSERT( f.m_found.nVal == 100 );
                CPPUNIT_ASSERT( f.m_found.stat.nEnsureExistFuncCall == 0 );
                CPPUNIT_ASSERT( f.m_found.stat.nEnsureNewFuncCall == 0 );
            }
            std::pair<bool, bool> ensureResult = s.update( key, ensure_functor() );
            CPPUNIT_ASSERT( ensureResult.first && !ensureResult.second );
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 3 ) );
            {
                copy_found<value_type> f;
                CPPUNIT_ASSERT( s.find( key, std::ref( f ) ) );
                CPPUNIT_ASSERT( f.m_found.nVal == 100 );
                CPPUNIT_ASSERT( f.m_found.stat.nEnsureExistFuncCall == 1 );
                CPPUNIT_ASSERT( f.m_found.stat.nEnsureNewFuncCall == 0 );
            }

            ensureResult = s.update( 13, []( bool /*bNew*/, key_type key, value_type& v ) 
                { 
                    v.nVal = key * 100; 
                    ++v.stat.nEnsureExistFuncCall; 
                });
            CPPUNIT_ASSERT( ensureResult.first && ensureResult.second );
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 4 ) );
            {
                copy_found<value_type> f;
                key = 13;
                CPPUNIT_ASSERT( s.find( key, std::ref( f ) ) );
                CPPUNIT_ASSERT( f.m_found.nVal == 1300 );
                CPPUNIT_ASSERT( f.m_found.stat.nEnsureExistFuncCall == 0 );
                CPPUNIT_ASSERT( f.m_found.stat.nEnsureNewFuncCall == 1 );
            }

            // erase test
            CPPUNIT_ASSERT( s.erase( 13 ) );
            CPPUNIT_ASSERT( !s.find( 13 ) );
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 3 ) );
            CPPUNIT_ASSERT( !s.erase( 13 ) );
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 3 ) );

            CPPUNIT_ASSERT( s.find( 10 ) );
            CPPUNIT_ASSERT( s.erase_with( 10, std::less<key_type>() ) );
            CPPUNIT_ASSERT( !s.find( 10 ) );
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 2 ) );
            CPPUNIT_ASSERT( !s.erase_with( 10, std::less<key_type>() ) );
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 2 ) );

            CPPUNIT_ASSERT( s.find( 20 ) );
            {
                copy_found<value_type> f;
                CPPUNIT_ASSERT( s.erase( 20, std::ref( f ) ) );
                CPPUNIT_ASSERT( f.m_found.nVal == 25 );

                CPPUNIT_ASSERT( s.insert( 235, 2350 ) );
                CPPUNIT_ASSERT( s.erase_with( 235, std::less<key_type>(), std::ref( f ) ) );
                CPPUNIT_ASSERT( f.m_found.nVal == 2350 );
            }
            CPPUNIT_ASSERT( !s.find( 20 ) );
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 1 ) );

            s.clear();
            CPPUNIT_ASSERT( s.empty() );
            CPPUNIT_ASSERT( check_size( s, 0 ) );

            // emplace test
            CPPUNIT_ASSERT( s.emplace( 151 ) );  // key = 151, val=0
            CPPUNIT_ASSERT( s.emplace( 174, 471 ) );    // key = 174, val = 471
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, 2 ) );

            CPPUNIT_ASSERT( s.find( 151 ) );
            CPPUNIT_ASSERT( s.find_with( 174, std::less<key_type>() ) );
            CPPUNIT_ASSERT( !s.find( 190 ) );

            {
                copy_found<value_type> f;
                key = 151;
                CPPUNIT_ASSERT( s.find( key, std::ref( f ) ) );
                CPPUNIT_ASSERT( f.m_found.nVal == 0 );

                key = 174;
                CPPUNIT_ASSERT( s.find( key, std::ref( f ) ) );
                CPPUNIT_ASSERT( f.m_found.nVal == 471 );
            }

            s.clear();
            CPPUNIT_ASSERT( s.empty() );
            CPPUNIT_ASSERT( check_size( s, 0 ) );
        }

        template <typename Set>
        void fill_set( Set& s, data_array& a )
        {
            CPPUNIT_ASSERT( s.empty() );
            for ( size_t i = 0; i < c_nItemCount; ++i ) {
                CPPUNIT_ASSERT( s.insert( a[i] ) );
            }
            CPPUNIT_ASSERT( !s.empty() );
            CPPUNIT_ASSERT( check_size( s, c_nItemCount ) );
        }

        template <class Set, class PrintStat>
        void test()
        {
            typedef Set set_type;

            set_type s;

            test_with( s );

            s.clear();
            CPPUNIT_ASSERT( s.empty() );
            CPPUNIT_ASSERT( check_size( s, 0 ) );



            PrintStat()(s);
        }


        void BronsonAVLTree_rcu_gpb_less();
        void BronsonAVLTree_rcu_gpb_cmp();
        void BronsonAVLTree_rcu_gpb_cmpless();
        void BronsonAVLTree_rcu_gpb_less_ic();
        void BronsonAVLTree_rcu_gpb_cmp_ic();
        void BronsonAVLTree_rcu_gpb_less_stat();
        void BronsonAVLTree_rcu_gpb_cmp_ic_stat();
        void BronsonAVLTree_rcu_gpb_cmp_ic_stat_yield();

        CPPUNIT_TEST_SUITE( BronsonAVLTreeHdrTest )
            CPPUNIT_TEST( BronsonAVLTree_rcu_gpb_less )
            CPPUNIT_TEST( BronsonAVLTree_rcu_gpb_cmp )
            CPPUNIT_TEST( BronsonAVLTree_rcu_gpb_cmpless )
            CPPUNIT_TEST( BronsonAVLTree_rcu_gpb_less_ic )
            CPPUNIT_TEST( BronsonAVLTree_rcu_gpb_cmp_ic )
            CPPUNIT_TEST( BronsonAVLTree_rcu_gpb_less_stat )
            CPPUNIT_TEST( BronsonAVLTree_rcu_gpb_cmp_ic_stat )
            CPPUNIT_TEST( BronsonAVLTree_rcu_gpb_cmp_ic_stat_yield )
        CPPUNIT_TEST_SUITE_END()
    };
} // namespace tree

#endif // #ifndef CDSTEST_HDR_BRONSON_AVLTREE_MAP_H
