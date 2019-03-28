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

        static int PORT = 11000;

        DateTime T;
        string STATE = "IDLE";
        Label[] labels;


        public Form1()
        {
            InitializeComponent();
            UDPListener();
            timer1.Start();
            
        }

        private void UDPListener()
        {
            Task.Run(async () =>
            {
                using (var udpClient = new UdpClient(PORT))
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
        private void parseMessage(string msg) {

            if (msg.Contains(":"))
            {
                if (msg.StartsWith("ready:"))
                {
                    setTime( "- -:- -.-");
                    STATE = "IDLE";
                }
                if (msg.StartsWith("idle:"))
                {
                    //setTime("- -:- -.-");
                    string w = msg.Split(':')[1];
                    
                    setWeight(Convert.ToInt32(w));
                }
                if (msg.StartsWith("armed:"))
                {
                    setTime( "00:00.0");
                    STATE = "ARMED";

                    beep.BeginInvoke((a) => { beep.EndInvoke(a); }, null);

                }
                else if (msg.StartsWith("start:"))
                {

                    setTime( "00:00.0");
                    T = new DateTime(0);
                    STATE = "RUNNING";
                }
                else if (msg.StartsWith("end:"))
                {
                    STATE = "IDLE";
                    string ms = msg.Split(':')[1];
                    setTime( new DateTime(Convert.ToInt64(ms) * 10000L), Color.Violet);
                }
            }
        }


        private void broadcast(string text)
        {
            using (var udpClient = new UdpClient(9999))
            {
                var data = Encoding.UTF8.GetBytes(text);
                udpClient.Send(data, data.Length, "255.255.255.255", PORT);
            }
        }

        private void setTime( string txt)
        {
            setTime( txt, Color.Red);
        }

        private void setTime(string txt, Color c )
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

        private void setTime(DateTime t, Color c)
        {
            setTime(t.ToString("mm:ss.f"), c );
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
            broadcast("start:");
        }
        private void button2_Click(object sender, EventArgs e)
        {
            broadcast("end:2345");
        }

        private void button3_Click(object sender, EventArgs e)
        {
            broadcast("armed:");
        }
        
        private void button4_Click(object sender, EventArgs e)
        {
            broadcast("ready:");
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (STATE == "RUNNING")
            {

                //Console.WriteLine(DateTime.Now.ToString("mm:ss.f"));
                setTime(T.ToString("mm:ss.f"));
                T = T.AddMilliseconds(timer1.Interval);
            }

        }
    }//end class
}//end namespace
