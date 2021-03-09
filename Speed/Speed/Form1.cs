using System;
using System.Drawing;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Speed
{
    public partial class Form1 : Form
    {
        static string ID = "0";
        static int LOCAL_PORT  = 11000;
        //static int REMOTE_PORT = 10999;

        DateTime LastAliveFromScale;
        DateTime t1;
        string STATE = "IDLE";

        public Form1()
        {
            InitializeComponent();
            UDPListener();
            timer1.Interval = 1;
            timer1.Start();
        }

        private void UDPListener()
        {

            Task.Run(async () =>
            {
                using (var udpClient = new UdpClient(LOCAL_PORT))
                {
                    while (true)
                    {
                        //IPEndPoint object will allow us to read datagrams sent from any source.
                        var receivedResults = await udpClient.ReceiveAsync();
                        string msg = Encoding.UTF8.GetString(receivedResults.Buffer);
                        Console.WriteLine(msg);
                        parseMessage(msg);
                    }
                }
            });
        }



        Action beep = Console.Beep;
        private void parseMessage(string text) {

            string[] words = text.Split(':');

            if (words.Length>1 && words[0]==ID)
            {

                string message = words[1];

                if (message.Equals("ready"))
                {
                    STATE = "IDLE";

                    setTime( "- -:- -.--");
                    
                }

                if (message.Equals("idle"))
                {
                   

                    //string w = msg.Split(':')[1];
                    setWeight(Convert.ToInt32(words[2]));

                    
                }
                if (message.Equals("armed"))
                {

                    
                    STATE = "ARMED";

                    setTime("00:00.00", Color.Red);
                    beep.BeginInvoke((a) => { beep.EndInvoke(a); }, null);

                    
                }
                else if (message.Equals("start"))
                {
                    t1 = DateTime.Now;
                    STATE = "RUNNING";

                    setTime( "00:00.00", Color.Red);
                    
                    
                }
                else if (message.Equals("stop") && STATE=="RUNNING")
                {
                   
                    STATE = "IDLE";
                    Console.WriteLine("Time of run<"+ words[2] + ">");
                    long ms = 0;
                    long.TryParse(words[2], out ms);

                    setTime(ms);
                    setTimeColor(Color.Yellow);
                }
            }
        }


        private void broadcast(string message, int port)
        {
            using (var udpClient = new UdpClient())
            {
                message = ID + ":" + message;
                var data = Encoding.UTF8.GetBytes(message);
                udpClient.Send(data, data.Length, "192.168.1.255", port);
            }
        }

        private void broadcast(string text)
        {
            broadcast(text, LOCAL_PORT);
        }

        private void setTime(string txt )
        {
            //txt = DateTime.Now.ToString("ss:ff");
            Label label = label1;
            if (label.InvokeRequired)
            {
                label.Invoke(new Action(() => label.Text = txt));
                return;
            }
            label.Text = txt;
        }

        private void setTime(string txt, Color c)
        {
            //txt = DateTime.Now.ToString("ss:ff");
            Label label = label1;
            if (label.InvokeRequired)
            {
                label.Invoke(new Action(() => label.Text = txt));
                label.Invoke(new Action(() => label.ForeColor = c));
                return;
            }
            label.Text = txt;
            label.ForeColor = c;
        }

        private void setTime() {
            long ticks = (long)((DateTime.Now - t1).TotalMilliseconds* 1e4);
            setTime((new DateTime(ticks)).ToString("mm:ss.ff"));
        }

        private void setTime(long ms)
        {
            long ticks = ms * 10000;
            setTime((new DateTime(ticks)).ToString("mm:ss.ff"));
        }

        private void setTimeColor(Color c)
        {
            Label label = label1;
            if (label.InvokeRequired)
            {
                label.Invoke(new Action(() => label.ForeColor = c));
                return;
            }
            label.ForeColor = c;
        }

        private void setWeight(int w)
        {
            string txt = "" + w + " kg.";
            Color c = ( w > 20)?Color.Red:Color.Silver;
            Label label = label2;
            if (label.InvokeRequired)
            {
                label.Invoke(new Action(() => label.Text = txt));
                label.Invoke(new Action(() => label.ForeColor = c));
                return;
            }
            label.Text = txt;
            label.ForeColor = c;
        }

        private void button1_Click(object sender, EventArgs e)
        {

            broadcast("start:0");
        }
        private void button2_Click(object sender, EventArgs e)
        {
            broadcast("stop:1234");
            
        }

        private void button3_Click(object sender, EventArgs e)
        {
            broadcast("armed:0");
        }
        

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (STATE == "RUNNING")
            {
                setTime();

            }

            if ((DateTime.Now - LastAliveFromScale).TotalSeconds > 10)
            {
                //setScaleReady(false);
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            Random r = new Random();

            int w = r.Next(100);
            broadcast("idle:"+w);
        }

        private void label2_Click(object sender, EventArgs e)
        {

        }

        private void button5_Click(object sender, EventArgs e)
        {
            broadcast("ready:0");

        }
    }//end class
}//end namespace
