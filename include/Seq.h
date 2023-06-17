#ifndef __Seq_h__
#define __Seq_h__

class Seq
    {
  public:
    static Seq * instance();

    class Task
        {
      public:
        Task();
        Task( const Task & anObj );
        Task & operator = ( const Task & anObj );
        ~Task();

        void link( Task & aNxt );
        Task * next();
        void unlink();

        /*
        ** The task object will be registered(reg) with the Seq class.
        */
        virtual void lp();
        virtual void stp();

      private:
        Task * ivNxt;
        Task * ivPrev;
        };

    void lp();
    void reg( Task & anObj );
    void stp();
    void unreg( Task & anObj );

  private:
    Seq();
    Seq( const Seq & anObj );
    Seq & operator = ( const Seq & anObj );
    ~Seq();

    Task ivHead;
    };

#endif
