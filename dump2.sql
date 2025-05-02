--
-- PostgreSQL database dump
--

-- Dumped from database version 16.8
-- Dumped by pg_dump version 16.8

-- Started on 2025-05-01 19:58:00

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- TOC entry 224 (class 1259 OID 16560)
-- Name: batch; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.batch (
    id_batch integer NOT NULL,
    batch_number text NOT NULL
);


ALTER TABLE public.batch OWNER TO postgres;

--
-- TOC entry 223 (class 1259 OID 16559)
-- Name: batch_id_batch_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.batch ALTER COLUMN id_batch ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.batch_id_batch_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 220 (class 1259 OID 16531)
-- Name: division; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.division (
    id_division integer NOT NULL,
    name text NOT NULL
);


ALTER TABLE public.division OWNER TO postgres;

--
-- TOC entry 219 (class 1259 OID 16530)
-- Name: division_id_division_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.division ALTER COLUMN id_division ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.division_id_division_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 222 (class 1259 OID 16539)
-- Name: employee; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.employee (
    id_employee integer NOT NULL,
    "Combined" boolean NOT NULL,
    id_person integer NOT NULL,
    id_post integer NOT NULL,
    id_division integer NOT NULL
);


ALTER TABLE public.employee OWNER TO postgres;

--
-- TOC entry 215 (class 1259 OID 16514)
-- Name: person; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.person (
    id_person integer NOT NULL,
    name text NOT NULL,
    second_name text NOT NULL,
    middle_name text
);


ALTER TABLE public.person OWNER TO postgres;

--
-- TOC entry 216 (class 1259 OID 16521)
-- Name: employee_id_employee_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.person ALTER COLUMN id_person ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.employee_id_employee_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 221 (class 1259 OID 16538)
-- Name: employment_id_employment_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.employee ALTER COLUMN id_employee ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.employment_id_employment_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 234 (class 1259 OID 16618)
-- Name: manufacturer; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.manufacturer (
    id_manufacturer integer NOT NULL,
    name text NOT NULL
);


ALTER TABLE public.manufacturer OWNER TO postgres;

--
-- TOC entry 233 (class 1259 OID 16617)
-- Name: manufacturer_id_manufacturer_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.manufacturer ALTER COLUMN id_manufacturer ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.manufacturer_id_manufacturer_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 244 (class 1259 OID 16679)
-- Name: measurement; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.measurement (
    id_measurement bigint NOT NULL,
    value_measurement double precision NOT NULL,
    measurement_number integer DEFAULT 1 NOT NULL,
    datetime time without time zone NOT NULL,
    quality_protective_layer boolean NOT NULL,
    id_employee integer NOT NULL,
    id_product integer NOT NULL,
    id_device integer NOT NULL,
    id_place_measurement integer NOT NULL
);


ALTER TABLE public.measurement OWNER TO postgres;

--
-- TOC entry 240 (class 1259 OID 16647)
-- Name: measurement_characteristics; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.measurement_characteristics (
    id_measurement_characteristics integer NOT NULL,
    id_measuring_value integer NOT NULL,
    upper_measurement_range double precision NOT NULL,
    lower_measurement_range double precision NOT NULL,
    error_rate double precision NOT NULL,
    "id_JSON" text NOT NULL
);


ALTER TABLE public.measurement_characteristics OWNER TO postgres;

--
-- TOC entry 239 (class 1259 OID 16646)
-- Name: measurement_characteristics_id_measurement_characteristics_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.measurement_characteristics ALTER COLUMN id_measurement_characteristics ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.measurement_characteristics_id_measurement_characteristics_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 243 (class 1259 OID 16678)
-- Name: measurement_id_measurement_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.measurement ALTER COLUMN id_measurement ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.measurement_id_measurement_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 242 (class 1259 OID 16666)
-- Name: measuring_device; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.measuring_device (
    id_measuring_device integer NOT NULL,
    device_serial text NOT NULL,
    id_measuring_device_model integer NOT NULL,
    date_commissioning date NOT NULL,
    date_verification date NOT NULL,
    repair_date date
);


ALTER TABLE public.measuring_device OWNER TO postgres;

--
-- TOC entry 241 (class 1259 OID 16665)
-- Name: measuring_device_id_measuring_device_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.measuring_device ALTER COLUMN id_measuring_device ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.measuring_device_id_measuring_device_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 238 (class 1259 OID 16634)
-- Name: measuring_device_model; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.measuring_device_model (
    id_measuring_device_model integer NOT NULL,
    name text NOT NULL,
    id_manufacturer integer NOT NULL,
    weight integer NOT NULL,
    id_measurement_characteristics integer NOT NULL
);


ALTER TABLE public.measuring_device_model OWNER TO postgres;

--
-- TOC entry 237 (class 1259 OID 16633)
-- Name: measuring_device_model_id_measuring_device_model_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.measuring_device_model ALTER COLUMN id_measuring_device_model ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.measuring_device_model_id_measuring_device_model_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 230 (class 1259 OID 16594)
-- Name: measuring_point; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.measuring_point (
    id_measuring_point integer NOT NULL,
    point text NOT NULL
);


ALTER TABLE public.measuring_point OWNER TO postgres;

--
-- TOC entry 229 (class 1259 OID 16593)
-- Name: measuring_point_id_measuring_point_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.measuring_point ALTER COLUMN id_measuring_point ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.measuring_point_id_measuring_point_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 236 (class 1259 OID 16626)
-- Name: measuring_value; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.measuring_value (
    id_measuring_value integer NOT NULL,
    name text NOT NULL
);


ALTER TABLE public.measuring_value OWNER TO postgres;

--
-- TOC entry 235 (class 1259 OID 16625)
-- Name: measuring_value_id_measuring_value_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.measuring_value ALTER COLUMN id_measuring_value ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.measuring_value_id_measuring_value_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 232 (class 1259 OID 16602)
-- Name: place_measurement; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.place_measurement (
    id_place_measurement integer NOT NULL,
    upper_limit_thickness double precision NOT NULL,
    lower_limit_thickness double precision NOT NULL,
    id_product_type integer NOT NULL,
    id_measurement_point integer NOT NULL
);


ALTER TABLE public.place_measurement OWNER TO postgres;

--
-- TOC entry 231 (class 1259 OID 16601)
-- Name: place_measurement_id_place_measurement_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.place_measurement ALTER COLUMN id_place_measurement ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.place_measurement_id_place_measurement_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 218 (class 1259 OID 16523)
-- Name: post; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.post (
    id_post integer NOT NULL,
    name text NOT NULL
);


ALTER TABLE public.post OWNER TO postgres;

--
-- TOC entry 217 (class 1259 OID 16522)
-- Name: post_id_post_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.post ALTER COLUMN id_post ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.post_id_post_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 228 (class 1259 OID 16576)
-- Name: product; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.product (
    id_product integer NOT NULL,
    product_serial text NOT NULL,
    id_product_type integer NOT NULL,
    id_batch integer
);


ALTER TABLE public.product OWNER TO postgres;

--
-- TOC entry 227 (class 1259 OID 16575)
-- Name: product_id_product_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.product ALTER COLUMN id_product ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.product_id_product_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 226 (class 1259 OID 16568)
-- Name: product_type; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.product_type (
    id_product_type integer NOT NULL,
    name text NOT NULL
);


ALTER TABLE public.product_type OWNER TO postgres;

--
-- TOC entry 225 (class 1259 OID 16567)
-- Name: product_type_id_product_type_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.product_type ALTER COLUMN id_product_type ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.product_type_id_product_type_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- TOC entry 4955 (class 0 OID 16560)
-- Dependencies: 224
-- Data for Name: batch; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.batch (id_batch, batch_number) FROM stdin;
1	A123
2	B159
\.


--
-- TOC entry 4951 (class 0 OID 16531)
-- Dependencies: 220
-- Data for Name: division; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.division (id_division, name) FROM stdin;
1	сектор документации
2	сектор контроля изготовления
\.


--
-- TOC entry 4953 (class 0 OID 16539)
-- Dependencies: 222
-- Data for Name: employee; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.employee (id_employee, "Combined", id_person, id_post, id_division) FROM stdin;
1	f	1	1	1
2	f	2	2	2
4	t	3	2	1
5	f	4	2	1
3	t	3	3	1
\.


--
-- TOC entry 4965 (class 0 OID 16618)
-- Dependencies: 234
-- Data for Name: manufacturer; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.manufacturer (id_manufacturer, name) FROM stdin;
1	Автолактест
2	ВОСТОК-7
3	НПЦ КРОПУС
4	Константа
\.


--
-- TOC entry 4975 (class 0 OID 16679)
-- Dependencies: 244
-- Data for Name: measurement; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.measurement (id_measurement, value_measurement, measurement_number, datetime, quality_protective_layer, id_employee, id_product, id_device, id_place_measurement) FROM stdin;
11	200	1	14:30:00	t	1	1	3	1
12	300	1	14:40:00	t	1	1	3	2
13	200	1	14:50:00	t	1	1	3	3
14	100	1	10:30:00	t	2	2	5	1
15	200	1	10:35:50	t	2	2	5	2
16	150	1	10:40:00	t	2	2	5	3
17	300	1	12:00:00	t	3	3	4	1
18	450	1	12:10:00	f	3	3	4	2
19	159	1	12:15:00	f	3	3	4	3
20	201.4	2	13:20:00	t	5	3	5	3
\.


--
-- TOC entry 4971 (class 0 OID 16647)
-- Dependencies: 240
-- Data for Name: measurement_characteristics; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.measurement_characteristics (id_measurement_characteristics, id_measuring_value, upper_measurement_range, lower_measurement_range, error_rate, "id_JSON") FROM stdin;
1	1	500	100	0.02	nanometers
3	1	120	0.5	0.02	nanometers
2	1	500	2	0.03	nanometers
\.


--
-- TOC entry 4973 (class 0 OID 16666)
-- Dependencies: 242
-- Data for Name: measuring_device; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.measuring_device (id_measuring_device, device_serial, id_measuring_device_model, date_commissioning, date_verification, repair_date) FROM stdin;
3	1234567890	1	2025-01-01	2025-01-19	\N
4	1324567809	2	2025-01-01	2025-01-19	\N
7	5656565656	1	2025-02-02	2025-02-15	2025-02-05
5	0987654321	3	2025-01-01	2025-01-19	2025-01-29
6	1212121212	2	2025-02-02	2025-02-15	\N
\.


--
-- TOC entry 4969 (class 0 OID 16634)
-- Dependencies: 238
-- Data for Name: measuring_device_model; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.measuring_device_model (id_measuring_device_model, name, id_manufacturer, weight, id_measurement_characteristics) FROM stdin;
1	МТП-10П	1	450	1
2	МТ-2007	2	320	2
3	Константа К6Ц	4	250	3
\.


--
-- TOC entry 4961 (class 0 OID 16594)
-- Dependencies: 230
-- Data for Name: measuring_point; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.measuring_point (id_measuring_point, point) FROM stdin;
1	верхняя
2	средняя
3	нижняя
\.


--
-- TOC entry 4967 (class 0 OID 16626)
-- Dependencies: 236
-- Data for Name: measuring_value; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.measuring_value (id_measuring_value, name) FROM stdin;
1	микрометры
2	нанометры
\.


--
-- TOC entry 4946 (class 0 OID 16514)
-- Dependencies: 215
-- Data for Name: person; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.person (id_person, name, second_name, middle_name) FROM stdin;
1	Иван	Иванов	Иванович
2	Сергей	Сергеев	Сергеевич
3	Алексей	Попов	\N
4	Владимир	Петров	Игоревич
\.


--
-- TOC entry 4963 (class 0 OID 16602)
-- Dependencies: 232
-- Data for Name: place_measurement; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.place_measurement (id_place_measurement, upper_limit_thickness, lower_limit_thickness, id_product_type, id_measurement_point) FROM stdin;
1	300	100	1	1
2	400	100	1	2
3	300	100	1	3
4	200	50	2	1
5	300	50	2	2
6	300	50	2	3
7	350	70	3	1
8	350	70	3	2
9	350	70	3	3
10	500	150	4	1
11	500	150	4	2
12	500	150	4	3
\.


--
-- TOC entry 4949 (class 0 OID 16523)
-- Dependencies: 218
-- Data for Name: post; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.post (id_post, name) FROM stdin;
1	мастер участка
2	производитель работ
3	начальник участка
\.


--
-- TOC entry 4959 (class 0 OID 16576)
-- Dependencies: 228
-- Data for Name: product; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.product (id_product, product_serial, id_product_type, id_batch) FROM stdin;
1	A1234567890	1	1
2	B1324567809	2	1
3	C0987654321	3	2
4	D1212121212	4	2
5	F5656565656	4	2
6	L5467678435	1	\N
\.


--
-- TOC entry 4957 (class 0 OID 16568)
-- Dependencies: 226
-- Data for Name: product_type; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.product_type (id_product_type, name) FROM stdin;
1	ПВХ-труба
2	Стальная труба
3	Медная труба
4	Оцинкованная труба
\.


--
-- TOC entry 4981 (class 0 OID 0)
-- Dependencies: 223
-- Name: batch_id_batch_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.batch_id_batch_seq', 2, true);


--
-- TOC entry 4982 (class 0 OID 0)
-- Dependencies: 219
-- Name: division_id_division_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.division_id_division_seq', 2, true);


--
-- TOC entry 4983 (class 0 OID 0)
-- Dependencies: 216
-- Name: employee_id_employee_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.employee_id_employee_seq', 4, true);


--
-- TOC entry 4984 (class 0 OID 0)
-- Dependencies: 221
-- Name: employment_id_employment_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.employment_id_employment_seq', 5, true);


--
-- TOC entry 4985 (class 0 OID 0)
-- Dependencies: 233
-- Name: manufacturer_id_manufacturer_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.manufacturer_id_manufacturer_seq', 4, true);


--
-- TOC entry 4986 (class 0 OID 0)
-- Dependencies: 239
-- Name: measurement_characteristics_id_measurement_characteristics_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.measurement_characteristics_id_measurement_characteristics_seq', 3, true);


--
-- TOC entry 4987 (class 0 OID 0)
-- Dependencies: 243
-- Name: measurement_id_measurement_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.measurement_id_measurement_seq', 20, true);


--
-- TOC entry 4988 (class 0 OID 0)
-- Dependencies: 241
-- Name: measuring_device_id_measuring_device_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.measuring_device_id_measuring_device_seq', 7, true);


--
-- TOC entry 4989 (class 0 OID 0)
-- Dependencies: 237
-- Name: measuring_device_model_id_measuring_device_model_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.measuring_device_model_id_measuring_device_model_seq', 3, true);


--
-- TOC entry 4990 (class 0 OID 0)
-- Dependencies: 229
-- Name: measuring_point_id_measuring_point_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.measuring_point_id_measuring_point_seq', 3, true);


--
-- TOC entry 4991 (class 0 OID 0)
-- Dependencies: 235
-- Name: measuring_value_id_measuring_value_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.measuring_value_id_measuring_value_seq', 2, true);


--
-- TOC entry 4992 (class 0 OID 0)
-- Dependencies: 231
-- Name: place_measurement_id_place_measurement_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.place_measurement_id_place_measurement_seq', 12, true);


--
-- TOC entry 4993 (class 0 OID 0)
-- Dependencies: 217
-- Name: post_id_post_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.post_id_post_seq', 4, true);


--
-- TOC entry 4994 (class 0 OID 0)
-- Dependencies: 227
-- Name: product_id_product_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.product_id_product_seq', 6, true);


--
-- TOC entry 4995 (class 0 OID 0)
-- Dependencies: 225
-- Name: product_type_id_product_type_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.product_type_id_product_type_seq', 4, true);


--
-- TOC entry 4768 (class 2606 OID 16566)
-- Name: batch batch_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.batch
    ADD CONSTRAINT batch_pkey PRIMARY KEY (id_batch);


--
-- TOC entry 4764 (class 2606 OID 16537)
-- Name: division division_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.division
    ADD CONSTRAINT division_pkey PRIMARY KEY (id_division);


--
-- TOC entry 4760 (class 2606 OID 16520)
-- Name: person employee_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.person
    ADD CONSTRAINT employee_pkey PRIMARY KEY (id_person);


--
-- TOC entry 4766 (class 2606 OID 16543)
-- Name: employee employment_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.employee
    ADD CONSTRAINT employment_pkey PRIMARY KEY (id_employee);


--
-- TOC entry 4778 (class 2606 OID 16624)
-- Name: manufacturer manufacturer_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.manufacturer
    ADD CONSTRAINT manufacturer_pkey PRIMARY KEY (id_manufacturer);


--
-- TOC entry 4784 (class 2606 OID 16653)
-- Name: measurement_characteristics measurement_characteristics_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measurement_characteristics
    ADD CONSTRAINT measurement_characteristics_pkey PRIMARY KEY (id_measurement_characteristics);


--
-- TOC entry 4788 (class 2606 OID 16684)
-- Name: measurement measurement_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measurement
    ADD CONSTRAINT measurement_pkey PRIMARY KEY (id_measurement);


--
-- TOC entry 4782 (class 2606 OID 16640)
-- Name: measuring_device_model measuring_device_model_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measuring_device_model
    ADD CONSTRAINT measuring_device_model_pkey PRIMARY KEY (id_measuring_device_model);


--
-- TOC entry 4786 (class 2606 OID 16672)
-- Name: measuring_device measuring_device_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measuring_device
    ADD CONSTRAINT measuring_device_pkey PRIMARY KEY (id_measuring_device);


--
-- TOC entry 4774 (class 2606 OID 16600)
-- Name: measuring_point measuring_point_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measuring_point
    ADD CONSTRAINT measuring_point_pkey PRIMARY KEY (id_measuring_point);


--
-- TOC entry 4780 (class 2606 OID 16632)
-- Name: measuring_value measuring_value_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measuring_value
    ADD CONSTRAINT measuring_value_pkey PRIMARY KEY (id_measuring_value);


--
-- TOC entry 4776 (class 2606 OID 16606)
-- Name: place_measurement place_measurement_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.place_measurement
    ADD CONSTRAINT place_measurement_pkey PRIMARY KEY (id_place_measurement);


--
-- TOC entry 4762 (class 2606 OID 16527)
-- Name: post post_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.post
    ADD CONSTRAINT post_pkey PRIMARY KEY (id_post);


--
-- TOC entry 4772 (class 2606 OID 16582)
-- Name: product product_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.product
    ADD CONSTRAINT product_pkey PRIMARY KEY (id_product);


--
-- TOC entry 4770 (class 2606 OID 16574)
-- Name: product_type product_type_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.product_type
    ADD CONSTRAINT product_type_pkey PRIMARY KEY (id_product_type);


--
-- TOC entry 4789 (class 2606 OID 16554)
-- Name: employee fk_employment_division; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.employee
    ADD CONSTRAINT fk_employment_division FOREIGN KEY (id_division) REFERENCES public.division(id_division);


--
-- TOC entry 4790 (class 2606 OID 16544)
-- Name: employee fk_employment_employee; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.employee
    ADD CONSTRAINT fk_employment_employee FOREIGN KEY (id_person) REFERENCES public.person(id_person);


--
-- TOC entry 4791 (class 2606 OID 16549)
-- Name: employee fk_employment_post; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.employee
    ADD CONSTRAINT fk_employment_post FOREIGN KEY (id_post) REFERENCES public.post(id_post);


--
-- TOC entry 4796 (class 2606 OID 16641)
-- Name: measuring_device_model fk_measuring_device_model_manufacturer; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measuring_device_model
    ADD CONSTRAINT fk_measuring_device_model_manufacturer FOREIGN KEY (id_manufacturer) REFERENCES public.manufacturer(id_manufacturer);


--
-- TOC entry 4794 (class 2606 OID 16612)
-- Name: place_measurement fk_place_measurement_measurement_point; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.place_measurement
    ADD CONSTRAINT fk_place_measurement_measurement_point FOREIGN KEY (id_measurement_point) REFERENCES public.measuring_point(id_measuring_point);


--
-- TOC entry 4795 (class 2606 OID 16607)
-- Name: place_measurement fk_place_measurement_product_type; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.place_measurement
    ADD CONSTRAINT fk_place_measurement_product_type FOREIGN KEY (id_product_type) REFERENCES public.product_type(id_product_type);


--
-- TOC entry 4792 (class 2606 OID 16588)
-- Name: product fk_product_batch; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.product
    ADD CONSTRAINT fk_product_batch FOREIGN KEY (id_batch) REFERENCES public.batch(id_batch);


--
-- TOC entry 4793 (class 2606 OID 16583)
-- Name: product fk_product_product_type; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.product
    ADD CONSTRAINT fk_product_product_type FOREIGN KEY (id_product_type) REFERENCES public.product_type(id_product_type);


--
-- TOC entry 4798 (class 2606 OID 16659)
-- Name: measurement_characteristics measurement_characteristics_id_measuring_value_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measurement_characteristics
    ADD CONSTRAINT measurement_characteristics_id_measuring_value_fkey FOREIGN KEY (id_measuring_value) REFERENCES public.measuring_value(id_measuring_value);


--
-- TOC entry 4800 (class 2606 OID 16695)
-- Name: measurement measurement_id_device_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measurement
    ADD CONSTRAINT measurement_id_device_fkey FOREIGN KEY (id_device) REFERENCES public.measuring_device(id_measuring_device);


--
-- TOC entry 4801 (class 2606 OID 16685)
-- Name: measurement measurement_id_employment_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measurement
    ADD CONSTRAINT measurement_id_employment_fkey FOREIGN KEY (id_employee) REFERENCES public.employee(id_employee);


--
-- TOC entry 4802 (class 2606 OID 16690)
-- Name: measurement measurement_id_product_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measurement
    ADD CONSTRAINT measurement_id_product_fkey FOREIGN KEY (id_product) REFERENCES public.product(id_product);


--
-- TOC entry 4799 (class 2606 OID 16700)
-- Name: measuring_device measuring_device_id_measuring_device_model_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measuring_device
    ADD CONSTRAINT measuring_device_id_measuring_device_model_fkey FOREIGN KEY (id_measuring_device_model) REFERENCES public.measuring_device_model(id_measuring_device_model) NOT VALID;


--
-- TOC entry 4797 (class 2606 OID 16705)
-- Name: measuring_device_model measuring_device_model_id_measurement_characteristics_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.measuring_device_model
    ADD CONSTRAINT measuring_device_model_id_measurement_characteristics_fkey FOREIGN KEY (id_measurement_characteristics) REFERENCES public.measurement_characteristics(id_measurement_characteristics) NOT VALID;


-- Completed on 2025-05-01 19:58:00

--
-- PostgreSQL database dump complete
--

